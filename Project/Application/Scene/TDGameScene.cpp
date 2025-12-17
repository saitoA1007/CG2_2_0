#include"TDGameScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"SpriteRenderer.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
#include"LogManager.h"
#include"CollisionConfig.h"
#include"AudioManager.h"
#include<numbers>
#include<algorithm>

#include"Application/CollisionTypeID.h"
#include"Extension/CustomRenderer.h"
#include"FPSCounter.h"
#include"Application/Player/Effect/PlayerDamageNumberEffect.h"

using namespace GameEngine;

TDGameScene::~TDGameScene() {
}

void TDGameScene::Initialize(SceneContext* context) {
	// ゲームシーンに必要な低レイヤー機能
#pragma region SceneSystem 
	// エンジン機能を取得
	context_ = context;

	// 登録するパラメータを設定
	GameParamEditor::GetInstance()->SetActiveScene("TDGameScene");

	// 初期化されたことのログを出す
	Log("Initialize : TDGameScene", "TDGameScene");

	// デバック用描画の初期化
	debugRenderer_ = DebugRenderer::Create();

	// 当たり判定の管理システムを初期化
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->ClearList();

	// メインカメラの初期化
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720, context_->graphicsDevice->GetDevice());
#pragma endregion

	// 入力コマンドを設定する
	InputRegisterCommand();

	// 天球モデルを生成
	skyDomeModel_ = context_->modelManager->GetNameByModel("SkyDome");
	skyDomeModel_->SetDefaultColor({ 1.0f,1.0f,1.0f,1.0f });
	skyDomeWorldTransform_.Initialize({{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}});

	// 地面を生成
	// 氷の地面を描画するためのモデル
	icePlaneModel_ = context_->modelManager->GetNameByModel("PlaneXZ");
	// 地面を生成する
	terrain_ = std::make_unique<Terrain>();
	terrain_->Initialize(context_->textureManager->GetHandleByName("gras.png"),
		context_->textureManager->GetHandleByName("ice.png"), context_->textureManager->GetHandleByName("iceNormal.png"));

	// 背景の岩モデルを生成する
	bgIceRockModel_ = context_->modelManager->GetNameByModel("IceRock");
	bgIceRockModel_->SetDefaultIsEnableLight(true);
	bgRockModel_  = context_->modelManager->GetNameByModel("Rock");
	bgRockModel_->SetDefaultIsEnableLight(true);
	// 背景の岩オブジェクトの初期化
	bgRock_ = std::make_unique<BgRock>();
	bgRock_->Initialize(bgIceRockModel_->GetDefaultTexture());

	// ライトの生成
	sceneLightingController_ = std::make_unique<SceneLightingController>();
	sceneLightingController_->Initialize(context_->graphicsDevice->GetDevice());

	// 壁を生成する
	wallModel_ = context_->modelManager->GetNameByModel("Wall");
	// ステージを生成を初期化
	stageManager_ = std::make_unique<StageManager>();
	stageManager_->Initialize(0);
	stageManager_->Update({},{});

	// StageWallPlane用モデル
	stageWallPlaneModel_ = context_->modelManager->GetNameByModel("PlaneXZ");

	// ステージ用の板ポリ壁用のマテリアルを生成し、地面のテクスチャをコピー
	stageWallPlaneMaterial_ = std::make_unique<IceMaterial>();
	stageWallPlaneMaterial_->Initialize();
	{
		IceMaterial* terrainMat = terrain_->GetMaterial();
		if (terrainMat) {
			stageWallPlaneMaterial_->materialData_->baseTextureHandle = terrainMat->materialData_->baseTextureHandle;
			stageWallPlaneMaterial_->materialData_->textureHandle = terrainMat->materialData_->textureHandle;
			stageWallPlaneMaterial_->materialData_->normalTextureHandle = terrainMat->materialData_->normalTextureHandle;
			stageWallPlaneMaterial_->materialData_->baseColor = terrainMat->materialData_->baseColor;
			stageWallPlaneMaterial_->materialData_->color = terrainMat->materialData_->color;
			stageWallPlaneMaterial_->materialData_->specularColor = terrainMat->materialData_->specularColor;
			stageWallPlaneMaterial_->materialData_->shininess = terrainMat->materialData_->shininess;
		}
	}
#ifdef _DEBUG
    StageWallPlane::RegisterDebugParam(stageWallPlaneMaterial_.get());
#else
    StageWallPlane::ApplyDebugParam(stageWallPlaneMaterial_.get());
#endif

	// StageWallPlaneを6つ初期化
	{
		// StageManager の Wall オブジェクトに合わせて StageWallPlane の変換を揃える
		auto& walls = stageManager_->GetWalls();
		size_t count = std::min(stageWallPlanes_.size(), walls.size());
		for (size_t i = 0; i < count; ++i) {
			// Wall と同じ変換を使いつつ、Y 回転を 180 度回して向きを反転させる
			const Transform& wallTransform = walls[i]->GetWorldTransform().transform_;
			Transform t = wallTransform;
			// Y 回転に PI を加えて向きを反転
			t.rotate.y += static_cast<float>(std::numbers::pi);
			// XZ 平面モデルを壁として使うために回転を調整
            t.rotate.x -= static_cast<float>(std::numbers::pi) / 2.0f;
			t.scale.z = 32.0f;
			t.translate.y = 16.0f;
			stageWallPlanes_[i].Initialilze(t);

			// 壁の向きに合わせた移動範囲制限用の大きな OBB コライダーを作成
			boundaryColliders_[i] = std::make_unique<GameEngine::OBBCollider>();
			// コライダーの位置を板ポリの中心に合わせる
			boundaryColliders_[i]->SetWorldPosition(wallTransform.translate);
			// 壁のスケールに基づいて、縦方向に大きく、横方向は適切な範囲を覆うサイズを設定
			Vector3 colliderSize = { wallTransform.scale.x, kBoundaryHalfHeight, wallTransform.scale.z * 0.5f };
			boundaryColliders_[i]->SetSize(colliderSize);
			// 壁の回転を使ってコライダーの向きを設定
            boundaryColliders_[i]->UpdateOrientationsFromRotate(wallTransform.rotate);
			// 地形属性に設定してプレイヤーが当たるようにする
			boundaryColliders_[i]->SetCollisionAttribute(kCollisionAttributeTerrain);
			boundaryColliders_[i]->SetCollisionMask(~kCollisionAttributeTerrain);
			UserData data;
            data.typeID = static_cast<uint32_t>(CollisionTypeID::BoundaryWall);
			boundaryColliders_[i]->SetUserData(data);
		}
		// 壁の数より StageWallPlane のエントリ数が多い場合、残りは見えないスケールで初期化
		for (size_t i = count; i < stageWallPlanes_.size(); ++i) {
			Transform t; // デフォルトの単位変換
			t.scale = { 0.0f,0.0f,0.0f };
			stageWallPlanes_[i].Initialilze(t);
			boundaryColliders_[i] = std::make_unique<GameEngine::OBBCollider>();
			boundaryColliders_[i]->SetWorldPosition(t.translate);
			boundaryColliders_[i]->SetSize({0.0f,0.0f,0.0f});
		}
	}

	// プレイヤーモデルを生成
	playerModel_ = context_->modelManager->GetNameByModel("Player");
	playerModel_->SetDefaultIsEnableLight(true);
	// プレイヤー用アニメーターを作成
    playerAnimator_ = std::make_unique<Animator>();
	// プレイヤークラスを初期化
	player_ = std::make_unique<Player>();
	playerAnimationData_[static_cast<size_t>(PlayerAnimationType::Walk)] = context_->animationManager->GetNameByAnimations("PlayerWalk");
	playerAnimationData_[static_cast<size_t>(PlayerAnimationType::Rush)] = context_->animationManager->GetNameByAnimations("PlayerRush");
	playerAnimationData_[static_cast<size_t>(PlayerAnimationType::DownAttack)] = context_->animationManager->GetNameByAnimations("PlayerDownAttack");
    playerAnimationData_[static_cast<size_t>(PlayerAnimationType::AirMove)] = context_->animationManager->GetNameByAnimations("PlayerAirMove");
    playerAnimator_->Initialize(playerModel_, &playerAnimationData_[static_cast<size_t>(PlayerAnimationType::Walk)]["歩き"]);
	player_->Initialize(playerAnimator_.get(), playerAnimationData_);
	
    // プレイヤーのエフェクト初期化
    playerChargeEffect_ = std::make_unique<PlayerChargeEffect>();
    playerChargeEffect_->Initialize();
    playerChargeEffect_->SetParent(&player_->GetWorldTransform());

    playerRushEffect_ = std::make_unique<PlayerRushEffect>();
    playerRushEffect_->Initialize();
    playerRushEffect_->SetParent(&player_->GetWorldTransform());

    playerAttackDownEffect_ = std::make_unique<PlayerAttackDownEffect>();
    playerAttackDownEffect_->Initialize();
    playerAttackDownEffect_->SetParent(&player_->GetWorldTransform());

	// プレイヤーの攻撃演出
	playerAttackEffect_ = std::make_unique<PlayerAttackEffect>();
	playerAttackEffect_->Initialize(context_->textureManager->GetHandleByName("HitEffect.png"));

    // 着地演出
    playerLandingEffect_ = std::make_unique<PlayerLandingEffect>();
    playerLandingEffect_->Initialize();

    // 着地時にカメラシェイク実行とエフェクトを起動
    player_->SetOnLandHit([this]() {
		// 入力時の攻撃力を0.0f-1.0fに正規化
		float rawPower = player_->GetAttackDownPowerf();
		float minP = player_->GetAttackDownMinPower();
		float maxP = player_->GetAttackDownMaxPower();
		float normalized = 0.0f;
		float denom = maxP - minP;
		if (denom > 1e-6f) {
			normalized = std::clamp((rawPower - minP) / denom, 0.0f, 1.0f);
		}

		cameraController_->StartCameraShake(8.0f * normalized, 1.0f, 128.0f * normalized,
			[](const Vector3 &a, const Vector3 &b, float t) { return EaseInOutCubic(a, b, t); },
			CameraController::ShakeOrigin::TargetPosition,
            true, false, true, false);
        if (playerLandingEffect_) {
            playerLandingEffect_->Emitter(player_->GetWorldTransform().GetWorldPosition(), normalized);
        }
    });

	// カメラコントローラークラスを初期化
	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize();

	// プレイヤーの影
	playerShadow_ = std::make_unique<PlaneProjectionShadow>();
	playerShadow_->Initialize(&player_->GetWorldTransform());

	// 壁衝突時のカメラシェイク設定
	player_->SetOnWallHit([this]() {
		cameraController_->StartCameraShake(3.0f, 0.5f, 1000.0f,
            [](const Vector3 &a, const Vector3 &b, float t) { return Lerp(a, b, t); },
			CameraController::ShakeOrigin::TargetPosition,
			true, false, true, false);
	});

	// プレイヤーがダメージを受けたときのカメラシェイク
	player_->SetOnDamaged([this]() {
		// 小さめの振動でプレイヤー被弾を表現
		cameraController_->StartCameraShake(6.0f, 0.5f, 800.0f,
			[](const Vector3 &a, const Vector3 &b, float t) { return EaseInOutCubic(a, b, t); },
			CameraController::ShakeOrigin::CameraPosition,
			true, true, true, false);
	});

	// 敵の処理に関する初期化処理
#pragma region EnemySystem 

	// 氷柱のモデルを取得
	iceFallModel_ = context_->modelManager->GetNameByModel("IceFall");
	iceFallModel_->SetDefaultIsEnableLight(true);
	// 破壊演出用の氷柱モデル
	breakIceFallModel_ = context_->modelManager->GetNameByModel("IceFallEffect");
	
	// 敵の攻撃管理クラス
	enemyAttackManager_ = std::make_unique<EnemyAttackManager>();
	enemyAttackManager_->Initialize(context_->postEffectManager_, iceFallModel_->GetDefaultTexture(), context_->textureManager->GetHandleByName("break.png"));
	// ボス敵モデルを生成
	bossEnemyModel_ = context_->modelManager->GetNameByModel("Boss");
	bossEnemyModel_->SetDefaultColor({ 1.0f,1.0f,1.0f,1.0f });
	bossEnemyModel_->SetDefaultIsEnableLight(true);
	// Egg用モデルを取得（存在すれば）
	bossEggModel_ = context_->modelManager->GetNameByModel("BossEgg");
	bossEggModel_->SetDefaultColor({1.0f,1.0f,1.0f,1.0f});
	bossEggModel_->SetDefaultIsEnableLight(true);

	// 敵のアニメーションデータを取得する
    enemyAnimationData_[static_cast<size_t>(enemyAnimationType::Appearance)] = context_->animationManager->GetNameByAnimations("BossBird_Appearance_Animation");
	enemyAnimationData_[static_cast<size_t>(enemyAnimationType::BaseMove)] = context_->animationManager->GetNameByAnimations("BossBirdBaseMove");
	enemyAnimationData_[static_cast<size_t>(enemyAnimationType::IceBreath)] = context_->animationManager->GetNameByAnimations("BossBird.IceBreath");
	enemyAnimationData_[static_cast<size_t>(enemyAnimationType::Rush)] = context_->animationManager->GetNameByAnimations("BossBirdRush");
	enemyAnimationData_[static_cast<size_t>(enemyAnimationType::Scream)] = context_->animationManager->GetNameByAnimations("BossBirdScream");
	enemyAnimationData_[static_cast<size_t>(enemyAnimationType::Death)] = context_->animationManager->GetNameByAnimations("ShootDown_Animation");

	// ボスのアニメーションの再生を管理する
	bossEnemyAnimator_ = std::make_unique<Animator>();
	bossEnemyAnimator_->Initialize(bossEnemyModel_, &enemyAnimationData_[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);

	// ボス敵クラスを初期化
	bossEnemy_ = std::make_unique<BossEnemy>();
	bossEnemy_->Initialize(stageManager_->GetRadius(), enemyAttackManager_.get(), bossEnemyAnimator_.get(), 
		&enemyAnimationData_,debugRenderer_.get());
	bossEnemy_->SetTexture(bossEnemyModel_->GetDefaultTexture(), context_->textureManager->GetHandleByName("perlinNoise.png"));
	
	// ボスの影
	bossEnemyShadow_ = std::make_unique<PlaneProjectionShadow>();
	bossEnemyShadow_->Initialize(&bossEnemy_->GetWorldTransform());

	// ボスの突進攻撃の初期化
	enemyRushEffect_ = std::make_unique<EnemyRushEffect>();
	enemyRushEffect_->Initialize(context_->textureManager->GetHandleByName("circle.png"));
	enemyRushEffect_->SetParent(&bossEnemy_->GetWorldTransform());

	// ボスの風攻撃の初期化
	enemyWindAttackParticle_ = std::make_unique<EnemyWindAttackParticle>();
	enemyWindAttackParticle_->Initialize(context_->textureManager->GetHandleByName("noise.png"),true);
	enemyWindShadowAttackParticle_ = std::make_unique<EnemyWindAttackParticle>();
	enemyWindShadowAttackParticle_->Initialize(context_->textureManager->GetHandleByName("noise.png"), false);

	// 突進攻撃演出モデル
	enemyRushModel_ = context_->modelManager->GetNameByModel("RushWave");
	// プレイヤー用エフェクトのモデルも同じものを使用
	playerChargeEffectModel_ = enemyRushModel_;
	playerRushEffectModel_ = enemyRushModel_;
	playerAttackDownEffectModel_ = enemyRushModel_;
	playerLandingEffectModel_ = enemyRushModel_;
	Material *material = playerAttackDownEffect_->GetMaterial();
	material->SetTextureHandle(playerAttackDownEffectModel_->GetDefaultTexture());
	for (auto &m : playerLandingEffect_->GetMaterials()) {
		m->SetTextureHandle(playerLandingEffectModel_->GetDefaultTexture());
    }
    // PlayerChargeEffect のマテリアルにもテクスチャを設定
    if (playerChargeEffect_ && playerChargeEffect_->GetMaterial()) {
        playerChargeEffect_->GetMaterial()->SetTextureHandle(playerChargeEffectModel_->GetDefaultTexture());
    }
	// 風攻撃演出モデル
	windModel_ = context_->modelManager->GetNameByModel("Wind");

	// 翼のモデルを取得する
	wingModel_ = context_->modelManager->GetNameByModel("Wing");

	// 翼の演出
	enemyWingsParticleParticle_ = std::make_unique<WingsParticle>();
	enemyWingsParticleParticle_->Initialize(wingModel_->GetDefaultTexture());
	//enemyWingsParticleParticle_->SetEmitterPos({ 0.0f,10.0f,0.0f });

	// 翼のパーティクル
	enterWingsParticle_ = std::make_unique<EnterWingsParticle>();
	enterWingsParticle_->Initialize(wingModel_->GetDefaultTexture());
	enterWingsParticle_->SetEmitterPos({ 0.0f,10.0f,0.0f });

	// 翼のバースト
	enterBurstWingsParticle_ = std::make_unique<EnterBurstWingsParticle>();
	enterBurstWingsParticle_->Initialize(wingModel_->GetDefaultTexture());
	enterBurstWingsParticle_->SetEmitterPos({ 0.0f,10.0f,0.0f });

	// ボスが常に纏っているパーティクル
	bossWearParticle_ = std::make_unique<ParticleBehavior>();
	bossWearParticle_->Initialize("BossWearParticle", 16);
	bossWearParticle_->Emit({ 0.0f,5.0f,0.0f });
	// 加算
	bossWearAdditionParticle_ = std::make_unique<ParticleBehavior>();
	bossWearAdditionParticle_->Initialize("BossWearAdditionParticle", 16);
	bossWearAdditionParticle_->Emit({ 0.0f,5.0f,0.0f });
#pragma endregion

	// 平面モデルを取得
	planeModel_ = context_->modelManager->GetNameByModel("Plane");

	//==================================================
	// カメラアニメーション設定
	//==================================================

	if (TDGameScene::IsFirstGameStart()) {
		//{
		//	// 指定のキーとイージング関数を設定する
		//	std::vector<AnimationKeyframe<Vector3>> positionKeys;
		//	std::vector<AnimationKeyframe<Vector3>> rotateKeys;
		//	std::vector<AnimationKeyframe<Vector3>> lookAtKeys;
		//	std::vector<AnimationKeyframe<float>> fovKeys;

		//	// イージングラッパー
		//	auto vecEase = [](const Vector3 &a, const Vector3 &b, float t) -> Vector3 { return EaseInOutCubic(a, b, t); };
		//	auto floatEase = [](const float &a, const float &b, float t) -> float { return EaseInOutCubic(a, b, t); };

		//	float pitch;
  //      	float yaw;
		//	const float degToRad = static_cast<float>(std::numbers::pi) / 180.0f;

		//	// t=0.5f のキー
  //      	pitch = -15.0f * degToRad;
		//	positionKeys.push_back(AnimationKeyframe<Vector3>{ 0.5f, Vector3{0.0f, 0.0f, 8.0f}, vecEase });
		//	rotateKeys.push_back(AnimationKeyframe<Vector3>{ 0.5f, Vector3{ pitch, 0.0f, 0.0f }, vecEase });
		//	lookAtKeys.push_back(AnimationKeyframe<Vector3>{ 0.5f, Vector3{ 0.0f, 0.0f, 0.0f }, vecEase });
		//	fovKeys.push_back(AnimationKeyframe<float>{ 0.5f, 0.7f, floatEase });

		//	// t=2.0f のキー
  //      	pitch = -45.0f * degToRad;
		//	yaw = 360.0f * degToRad;
		//	positionKeys.push_back(AnimationKeyframe<Vector3>{ 2.0f, Vector3{0.0f, 0.0f, 32.0f}, vecEase });
		//	rotateKeys.push_back(AnimationKeyframe<Vector3>{ 2.0f, Vector3{ pitch, yaw, 0.0f }, vecEase });
		//	lookAtKeys.push_back(AnimationKeyframe<Vector3>{ 2.0f, Vector3{ 0.0f, 0.0f, 0.0f }, vecEase });
		//	fovKeys.push_back(AnimationKeyframe<float>{ 2.0f, 1.5f, floatEase });

		//	cameraController_->SetAnimationKeyframes(positionKeys, rotateKeys, lookAtKeys, fovKeys);
		//	cameraController_->PlayAnimation();
		//	cameraController_->StartCameraShake(0.5f, 10.0f, 20.0f,
  //          [](const Vector3 &a, const Vector3 &b, float t) { return EaseInOutSine(a, b, t); },
		//		CameraController::ShakeOrigin::TargetPosition,
		//		false, true, true, true);
		//}
	}
	cameraController_->SetCurrentAsDesired();

	// 仮UI
	playGuideSprite_ = Sprite::Create({ 16.0f,580.0f }, { 300.0f,150.0f }, { 0.0f,0.0f });
	playGuideGH_ = context_->textureManager->GetHandleByName("playerGuide.png");

	// タイトルスプライトを追加（TitleSceneからコピー）
	titleSprite_ = Sprite::Create({ 640.0f,250.0f }, { 1024.0f,256.0f }, { 0.5f,0.5f });
	titleGH_ = context_->textureManager->GetHandleByName("titleText.png");

	spaceSprite_ = Sprite::Create({ 640.0f,500.0f }, { 256.0f,64.0f }, { 0.5f,0.5f });
	spaceGH_ = context_->textureManager->GetHandleByName("spaceText.png");

	// ボスのhpUIを初期化
	bossHpUI_ = std::make_unique<BossHpUI>();
	bossHpUI_->Initialize(bossEnemy_->GetMaxHp());
	bossNameGH_ = context_->textureManager->GetHandleByName("BossName.png");

	// プレイヤーのhpUIを初期化 (仮の最大HP: 3)
	playerHpUI_ = std::make_unique<PlayerHpUI>();
    playerHpUI_->Initialize(player_->GetMaxHP(), context_->textureManager);

	// TutorialUI 初期化
	tutorialUI_ = std::make_unique<TutorialUI>();
	if (tutorialUI_) {
		tutorialUI_->Initialize(context_->textureManager);
		for (auto &spr : tutorialUI_->GetSprites()) {
			spr->SetColor(Vector4(1.0f,1.0f,1.0f,0.0f));
		}
	}

    // プレイヤーダメージ数値エフェクトの初期化
    playerDamageNumberEffect_ = std::make_unique<PlayerDamageNumberEffect>();
    playerDamageNumberEffect_->Initialize(context_->textureManager);

	// ゲームオーバーUIの初期化
	gameOverUI_ = std::make_unique<GameOverUI>();
	gameOverUI_->Initialize(context_->input, context_->inputCommand, context_->textureManager);
	// リトライ・タイトルのコールバックを設定
	gameOverUI_->SetOnRetryClicked([this]() {
        nextSceneState_ = SceneState::TDGame;
        TDGameScene::SetIsFirstGameStart(false);
        AudioManager::GetInstance().Stop(titleBGMHandle_);
        AudioManager::GetInstance().Stop(gameBGMHandle_);
		isFinished_ = true;
	});
	gameOverUI_->SetOnTitleClicked([this]() {
		nextSceneState_ = SceneState::TDGame;
        TDGameScene::SetIsFirstGameStart(true);
        AudioManager::GetInstance().Stop(titleBGMHandle_);
        AudioManager::GetInstance().Stop(gameBGMHandle_);
		isFinished_ = true;
	});

	// 空気を演出するためのパーティクル
	airParticle_ = std::make_unique<ParticleBehavior>();
	airParticle_->Initialize("AirParticle", 128);
	airParticle_->Emit({ 0.0f,0.0f,0.0f });

	// プレイヤーの回復演出
	playerGetHeartParticle_ = std::make_unique<ParticleBehavior>();
	playerGetHeartParticle_->Initialize("PlayerHeartParticle", 16);
	playerGetHeartParticle_->Emit({ 10.0f,-10.0f,0.0f });
	playerGetHeartParticle_->SetIsLoop(false);

	// プレイヤーの歩く時の演出
	playerWalkParticle_ = std::make_unique<ParticleBehavior>();
	playerWalkParticle_->Initialize("PlayerWalkParticle", 32);
	playerWalkParticle_->Emit({ 10.0f,10.0f,0.0f });

	transitionStartTarget_ = Vector3{ -2.0f,32.0f,-16.0f };
	transitionEndTarget_ = player_ ? player_->GetWorldTransform().GetWorldPosition() : transitionStartTarget_;

	// cameraController のターゲットを初期注視点に設定して内部状態を同期
	cameraController_->SetTarget(transitionStartTarget_);
	cameraController_->SetCameraCoordinateType(CameraController::CameraCoodinateType::Spherical);
	cameraController_->SetDesiredFov(0.7f);
	cameraController_->Update(context_->inputCommand, context_->input);
	cameraController_->SetCurrentAsDesired();
	cameraController_->Update(context_->inputCommand, context_->input);
	mainCamera_->SetCamera(cameraController_->GetCamera());
	mainCamera_->Update();

	if (sIsFirstGameStart_) {
		isTitleLocked_ = true;
		isTransitioning_ = false;
		transitionTimer_ = 0.0f;
        // タイトル表示中はその他UIを非表示に（アルファ0）
        uiFadeAlpha_ = 0.0f;
        /*bossHpUI_->GetFrameSprite()->SetColor(Vector4(1.0f,1.0f,1.0f,0.0f));
        bossHpUI_->GetEffectSprite()->SetColor(Vector4(1.0f,1.0f,1.0f,0.0f));
        bossHpUI_->GetSprite()->SetColor(Vector4(1.0f,1.0f,1.0f,0.0f));
        bossHpUI_->GetNameSprite()->SetColor(Vector4(1.0f,1.0f,1.0f,0.0f));*/
        for (auto &s : playerHpUI_->GetHpSprites()) { s->SetColor(Vector4(1.0f,1.0f,1.0f,0.0f)); }
        if (playGuideSprite_) playGuideSprite_->SetColor(Vector4(1.0f,1.0f,1.0f,0.0f));
        if (tutorialUI_) {
            for (auto &spr : tutorialUI_->GetSprites()) { spr->SetColor(Vector4(1.0f,1.0f,1.0f,0.0f)); }
        }
    } else {
        isTitleLocked_ = false;
        titleSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
        spaceSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
        uiFadeAlpha_ = 1.0f;
        if (tutorialUI_) {
            for (auto &spr : tutorialUI_->GetSprites()) { spr->SetColor(Vector4(1.0f,1.0f,1.0f,1.0f)); }
        }
    }

    // Letterbox 初期化（高さ0で開始）
    letterbox_ = std::make_unique<Letterbox>();
    if (letterbox_) {
        float w = 1280.0f;
        float h = 720.0f;
        letterbox_->Initialize(w, h, 0.0f);
        letterbox_->SetBoxHeight(0.0f);
        letterboxAnimTimer_ = 0.0f;
        letterboxStartHeight_ = 0.0f;
        letterboxEndHeight_ = 0.0f;
    }

	// 回復用ハートモデルを取得
	heartModel_ = context_->modelManager->GetNameByModel("Heart");

	// クリアUI
	clearUI_ = std::make_unique<ClearUI>();
	clearUI_->Initialize(context_->inputCommand, context_->textureManager);
	clearUI_->SetBackTitle([this]() {
		nextSceneState_ = SceneState::TDGame;
		TDGameScene::SetIsFirstGameStart(true);
		AudioManager::GetInstance().Stop(titleBGMHandle_);
		AudioManager::GetInstance().Stop(gameBGMHandle_);
		isFinished_ = true;
	});

	// ボスの撃破時のフェード
	bossDestroyFade_ = std::make_unique<BossDestroyFade>();
	bossDestroyFade_->Initialize();

    titleBGMHandle_ = AudioManager::GetInstance().GetHandleByName("Title_BGM.mp3");
    gameBGMHandle_ = AudioManager::GetInstance().GetHandleByName("Game_BGM.mp3");
    decideSEHandle_ = AudioManager::GetInstance().GetHandleByName("Decide.mp3");
    selectSEHandle_ = AudioManager::GetInstance().GetHandleByName("Select.mp3");
    bossLockOnSEHandle_ = AudioManager::GetInstance().GetHandleByName("Camera_In.mp3");
    bossUnlockSEHandle_ = AudioManager::GetInstance().GetHandleByName("Camera_Out.mp3");
    bossEggDamageSEHandle_ = AudioManager::GetInstance().GetHandleByName("Egg_Damaged.mp3");
    bossEggCrackSEHandle_ = AudioManager::GetInstance().GetHandleByName("Egg_Clack_02.mp3");
    bossEggBreakSEHandle_ = AudioManager::GetInstance().GetHandleByName("Egg_Clack.mp3");
    bossScreamSEHandle_ = AudioManager::GetInstance().GetHandleByName("Scream.mp3");
    AudioManager::GetInstance().Play(titleBGMHandle_, 0.5f, true);

	// 初回実行フラグを解除
	TDGameScene::SetIsFirstGameStart(false);

	// 矢印
	arrowUI_ = std::make_unique<ArrowUI>();
	arrowUI_->Initialize(context_->textureManager->GetHandleByName("Mark.png"));

    // UI初期状態
    uiDisabledForIntro_ = false;
    prevBossIntroPlaying_ = false;
    uiPostIntroFadeTimer_ = 0.0f;

    // BossAppearanceUI を初期化して非アクティブで保持
    bossAppearanceUI_ = std::make_unique<BossAppearanceUI>();
    bossAppearanceUI_->Initialize(context_->textureManager);
    bossAppearanceUI_->SetActive(false);

    // ボスダメージ時コールバック
	bossEnemy_->SetOnDamaged([this]() {
        int attackPower = player_ ? player_->GetAttackDownPower() : 0;
		if (attackPower >= 15) {
			AudioManager::GetInstance().Play(bossEggBreakSEHandle_, 1.0f, false);
        }
    });
}

void TDGameScene::Update() {
	if (isTitleLocked_ && !isTransitioning_) {
		if (context_->inputCommand->IsCommandActive("Start")) {
			isTransitioning_ = true;
			transitionTimer_ = 0.0f;
		}
	}

	if (isTitleLocked_) {
		// 遷移中は注視点をイージングで変化させる
		if (isTransitioning_) {
			transitionTimer_ += GameEngine::FpsCounter::deltaTime;
			float t = std::clamp(transitionTimer_ / kTransitionDuration_, 0.0f, 1.0f);
			float eased = EaseInOutCubic(0.0f, 1.0f, t);
			Vector3 currentTarget = Lerp(transitionStartTarget_, transitionEndTarget_, eased);
			cameraController_->SetTarget(currentTarget);

			// スプライトのフェード（タイトルは1->0、その他UIは0->1）
			float titleAlpha = 1.0f - eased;
			uiFadeAlpha_ = eased;
			if (titleSprite_) titleSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, titleAlpha));
			if (spaceSprite_) spaceSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, titleAlpha));
			// UIフェードインを反映
            /*bossHpUI_->GetFrameSprite()->SetAlpha(uiFadeAlpha_);
            bossHpUI_->GetEffectSprite()->SetAlpha(uiFadeAlpha_);
            bossHpUI_->GetSprite()->SetAlpha(uiFadeAlpha_);
            bossHpUI_->GetNameSprite()->SetAlpha(uiFadeAlpha_);*/
			for (auto &s : playerHpUI_->GetHpSprites()) { s->SetColor(Vector4(1.0f,1.0f,1.0f, uiFadeAlpha_)); }
			if (playGuideSprite_) playGuideSprite_->SetColor(Vector4(1.0f,1.0f,1.0f, uiFadeAlpha_));
            // TutorialUI のフェードイン
            if (tutorialUI_) {
                for (auto &spr : tutorialUI_->GetSprites()) {
                    spr->SetColor(Vector4(1.0f,1.0f,1.0f, uiFadeAlpha_));
                }
            }

			if (transitionTimer_ >= kTransitionDuration_) {
				isTransitioning_ = false;
				isTitleLocked_ = false;
				uiFadeAlpha_ = 1.0f;
			}
		} else {
			// タイトル表示中はUIは完全に隠す
			uiFadeAlpha_ = 0.0f;
		}
	} else {
		// タイトル解除後はUIをフル表示
		uiFadeAlpha_ = 1.0f;
	}

	if (context_->input->TriggerKey(DIK_U)) {
		if (bossEnemy_->GetCurrentHP() == 0) {
			nextSceneState_ = SceneState::Result;
		}
	 isFinished_ = true;
	}

	// ボスへのヒット演出処理
	bool currentBossHit = bossEnemy_->IsHit();
	if (currentBossHit && !prevBossHit_) {
		isBossHitFreezeActive_ = true;
		bossHitFreezeTimer_ = kBossHitFreezeDuration;
		// Boss登場時演出の予約（Egg状態でプレイヤーの急降下攻撃を喰らった場合）
		if (bossEnemy_->GetBossState() == BossState::Egg && player_ && player_->IsAttackDown()) {
			bossIntroScheduled_ = true;
			bossIntroDelayTimer_ = 0.0f;
			bossIntroPlaying_ = false;
			bossIntroTimer_ = 0.0f;
			bossIntroDelayAfterFreeze_ = false;
			bossIntroFinished_ = false; // これから演出予定
            AudioManager::GetInstance().Play(bossEggCrackSEHandle_, 0.5f, false);
            AudioManager::GetInstance().Play(bossEggDamageSEHandle_, 2.0f, false);
		}
		if (cameraController_) {
			cameraController_->SetDesiredFov(1.0f);
			cameraController_->StartCameraShake(64.0f, kBossHitFreezeDuration, 256.0f,
				[](const Vector3 &a, const Vector3 &b, float t) { return EaseInOutCubic(a, b, t); },
				CameraController::ShakeOrigin::TargetAndCameraPosition,
				true, false, true, false);
		}
		// 攻撃演出のエミッター
		Vector3 pos = player_->GetPlayerPos();
		float ratio = player_->GetAttackDownPower() / player_->GetAttackDownMaxPower();
		pos.y += 3.0f;
		playerAttackEffect_->Emitter(pos, ratio);
		Vector3 bp = bossEnemy_->GetWorldPosition();
		bp.y += 2.0f;
		enemyAttackManager_->AddEnemyDestroyEffect(bp);

		// ダメージ数字表示
        int damageVal = player_ ? player_->GetAttackDownPower() : 0;
        Vector3 screenWorldPos = bossEnemy_->GetWorldPosition();
        screenWorldPos.y += 4.0f;
        if (playerDamageNumberEffect_) {
            playerDamageNumberEffect_->Emitter(screenWorldPos, damageVal, cameraController_->GetCamera());
        }
	}
	prevBossHit_ = currentBossHit;
    playerDamageNumberEffect_->Update();

	// フリーズ中の処理（ボスヒット演出）
	if (isBossHitFreezeActive_) {
		bossHitFreezeTimer_ -= GameEngine::FpsCounter::deltaTime;
		cameraController_->Update(context_->inputCommand, context_->input);
		mainCamera_->SetCamera(cameraController_->GetCamera());

		if (bossHitFreezeTimer_ <= 0.0f) {
			isBossHitFreezeActive_ = false;
			bossHitFreezeTimer_ = 0.0f;
			// フリーズ解除後から0.5秒の猶予をカウント開始
			if (bossIntroScheduled_) {
				bossIntroDelayAfterFreeze_ = true;
				bossIntroDelayTimer_ = 0.0f;
				// 猶予時間開始前にロックオンが有効なら無効化し、レターボックスを閉じる
				if (isBossLockOn_) {
					isBossLockOn_ = false;
					if (letterbox_) {
						letterboxAnimTimer_ = 0.0f;
						letterboxStartHeight_ = letterboxEndHeight_;
						letterboxEndHeight_ = 0.0f;
					}
				}
			}
		}

		return;
	}

	// フリーズ解除後の猶予時間をカウントし、0.5秒で登場演出開始
	if (bossIntroDelayAfterFreeze_) {
		bossIntroDelayTimer_ += GameEngine::FpsCounter::deltaTime;
		if (bossIntroDelayTimer_ >= kBossIntroStartDelay_) {
			bossIntroPlaying_ = true;
			bossIntroScheduled_ = false;
			bossIntroDelayAfterFreeze_ = false;
			bossIntroTimer_ = 0.0f;
			player_->Restart();
			cameraController_->EnableAutoRotate(false);
            AudioManager::GetInstance().Stop(titleBGMHandle_);
            // Letterbox を表示開始
            if (letterbox_) {
				letterboxAnimTimer_ = 0.0f;
				letterboxStartHeight_ = letterboxEndHeight_;
				letterboxEndHeight_ = 64.0f;
			}
		}
	}

	//==================================================
	// ボス登場時演出処理
    //==================================================

	if (bossIntroPlaying_) {
		float prev = bossIntroTimer_;
		bossIntroTimer_ += GameEngine::FpsCounter::deltaTime;
		if (bossEnemy_->GetBossState() == BossState::Egg && bossIntroTimer_ >= 1.0f) bossEnemy_->SetBossStateIn();

		Vector3 bossPos = bossEnemy_->GetWorldTransform().GetWorldPosition();
		// 斜め下からボスを見上げる構図: 目線を少し下へ
		Vector3 eye = { 0.0f, 1.0f, -15.0f };
		Vector3 center = bossPos;
		center.y += 4.0f;
		// Euler未指定でLookAtを使用（即時反映）
		Vector3 euler = { 0.0f, 0.0f, 0.0f };
		float fov = 1.0f; // 演出用にやや広角

		// Egg状態でタイマーが1.0f到達時に氷柱破壊パーティクルを発生させる（1回のみ）
		if (bossEnemy_->GetBossState() == BossState::Egg) {
            if (prev < 1.0f && bossIntroTimer_ >= 1.0f) {
                enemyAttackManager_->AddEnemyDestroyEffect(bossPos);
                enemyAttackManager_->AddEnemyDestroyEffect(center);
                AudioManager::GetInstance().Play(bossEggBreakSEHandle_, 0.5f, false);
            }
        }
		
		//--------- ボスが上に上がりきったとき ---------//
		if (bossIntroTimer_ >= 3.0f) {
            eye = { 0.0f, 10.0f, -30.0f };
		}
		cameraController_->ApplyImmediateView(eye, center, euler, fov);
		if (bossIntroTimer_ >= 5.0f && prev < 5.0f) {
			cameraController_->StartCameraShake(4.0f, 2.0f, 64.0f,
				[](const Vector3 &a, const Vector3 &b, float t) { return EaseInOutCubic(a, b, t); },
				CameraController::ShakeOrigin::TargetPosition,
                true, true, true, false);
            AudioManager::GetInstance().Play(bossScreamSEHandle_, 1.0f, false);
            // BossAppearanceUIを有効化
            if (bossAppearanceUI_) { bossAppearanceUI_->SetActive(true); }
		}

		//--------- アニメーション終了 ---------//
        if (bossIntroTimer_ >= kBossIntroDuration_) {
			bossIntroPlaying_ = false;
			bossIntroTimer_ = 0.0f;
			AudioManager::GetInstance().Play(gameBGMHandle_, 0.5f, true);
            uiDisabledForIntro_ = true;
			bossIntroFinished_ = true; // 演出完了
			cameraController_->EnableAutoRotate(true);
			// Letterbox を非表示に戻す
			if (letterbox_) {
				letterboxAnimTimer_ = 0.0f;
				letterboxStartHeight_ = letterboxEndHeight_;
				letterboxEndHeight_ = 0.0f;
			}
		}

		mainCamera_->SetCamera(cameraController_->GetCamera());
	}

	//==================================================
    // ボス撃破時演出処理
    //==================================================

	// 撃破時演出開始ディレイ処理
	if (bossOutroScheduled_ && !bossOutroPlaying_) {
		// レターボックスを表示開始
		if (letterbox_) {
			letterboxAnimTimer_ = 0.0f;
			letterboxStartHeight_ = letterboxEndHeight_;
			letterboxEndHeight_ = 64.0f;
		}
		bossOutroTimer_ = 0.0f;
		bossOutroPlaying_ = true;
		bossOutroScheduled_ = false;
		cameraController_->EnableAutoRotate(false);
	}

	// 撃破時演出の再生
	if (bossOutroPlaying_) {
		[[maybe_unused]] float prev = bossOutroTimer_;
		bossOutroTimer_ += GameEngine::FpsCounter::deltaTime;

		// カメラをボス位置へズームアウトするような動き
		Vector3 bossPos = bossEnemy_->GetWorldTransform().GetWorldPosition();
		Vector3 eye = { 0.0f, 6.0f, -24.0f };
		Vector3 center = bossPos;
		center.y += 6.0f;
		Vector3 euler = { 0.0f, 0.0f, 0.0f };
		float fov = 1.0f;

		//if (bossOutroTimer_ >= 4.0f) {
		//	// 少し引きの画に移行
		//	eye = { 0.0f, 12.0f, -28.0f };
		//	fov = 0.7f;
		//}
		cameraController_->ApplyImmediateView(eye, center, euler, fov);

		//if (bossOutroTimer_ >= kBossOutroDuration_) {
		//	bossOutroPlaying_ = false;
		//	bossOutroTimer_ = 0.0f;
		//	// レターボックスを閉じる
		//	if (letterbox_) {
		//		letterboxAnimTimer_ = 0.0f;
		//		letterboxStartHeight_ = letterboxEndHeight_;
		//		letterboxEndHeight_ = 0.0f;
		//	}
		//}

		mainCamera_->SetCamera(cameraController_->GetCamera());
		if (bossOutroTimer_ >= 6.0f) {
			if (!clearUI_->IsActive()) {
				clearUI_->SetIsActice(true);
			}
		}
	}

	// 時間を取得する
	if (bossEnemy_->Isnow()) {
		clearUI_->StartTime();
	}

	// デバックリストを削除
	debugRenderer_->Clear();

	// 当たり判定をリセット
	collisionManager_->ClearList();

	// ライトの更新処理
	sceneLightingController_->Update();

	// プレイヤーの更新処理
	if (isTitleLocked_ || bossIntroPlaying_ || bossOutroPlaying_ || !player_->IsAlive()) {
		player_->Update(nullptr, cameraController_->GetCamera());
	} else {
		player_->Update(context_->inputCommand, cameraController_->GetCamera());
	}
	// プレイヤーの影の更新処理
	playerShadow_->Update();

    // プレイヤーのエフェクト更新処理
    if (player_) {
        int lvl = player_->GetRushChargeLevel();
		playerChargeEffect_->SetLevel(lvl);
    }
	playerChargeEffect_->Update();
    playerRushEffect_->Update();
    playerAttackDownEffect_->Update();
	playerAttackEffect_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());
	playerLandingEffect_->Update();

	// 回復演出
	if (player_->IsHearted()) {
		// プレイヤーの回復演出
		playerGetHeartParticle_->SetEmitterPos(player_->GetPlayerPos() + Vector3(0.0f, 1.0f, 0.0f));
		playerGetHeartParticle_->SetIsLoop(true);
	} else {
		playerGetHeartParticle_->SetIsLoop(false);
	}
	playerGetHeartParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// プレイヤーの歩く演出
	if (player_->GetPlayerPos().y <= 1.0f) {
		playerWalkParticle_->SetIsLoop(true);
		playerWalkParticle_->SetEmitterPos(player_->GetPlayerPos());
		playerWalkParticle_->SetVelocity(player_->GetDir() * -1.0f);
	} else {
		playerWalkParticle_->SetIsLoop(false);
	}
	playerWalkParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

    // プレイヤーの攻撃力に応じてAttackDownエフェクトの色を変化させる
	if (player_ && playerAttackDownEffect_) {
		playerAttackDownEffect_->SetColorFromAttackPower(
			player_->GetAttackDownMinPower(),
			player_->GetAttackDownMaxPower(),
			player_->GetAttackDownPowerf()
		);
	}

	// ロックオン: 入力が有効ならプレイヤーとボスの位置をターゲットに設定
	bool prevLockOn = isBossLockOn_;
	if (!isTitleLocked_) {
		if (!bossIntroDelayAfterFreeze_ && !bossIntroPlaying_ && !bossOutroPlaying_) {
			if (context_->inputCommand->IsCommandActive("LockOnBoss")) {
				isBossLockOn_ = !isBossLockOn_;
				if (isBossLockOn_) {
					AudioManager::GetInstance().Play(bossLockOnSEHandle_, 0.25f, false);
				} else {
					AudioManager::GetInstance().Play(bossUnlockSEHandle_, 0.5f, false);
                }
			}
		}

		if (!bossIntroPlaying_ && !bossOutroPlaying_) {
			if (isBossLockOn_) {
				std::vector<Vector3> targets;
				targets.reserve(2);
				targets.emplace_back(player_->GetWorldTransform().GetWorldPosition());
				targets.emplace_back(bossEnemy_->GetWorldTransform().GetWorldPosition());
				cameraController_->SetTarget(targets);
			} else {
				cameraController_->SetTarget(player_->GetWorldTransform().GetWorldPosition());
			}
		}
	}
	if (prevLockOn != isBossLockOn_) {
		letterboxAnimTimer_ = 0.0f;
		letterboxStartHeight_ = letterboxEndHeight_;
		letterboxEndHeight_ = isBossLockOn_ ? 64.0f : 0.0f;
	}

    if (letterbox_) {
        letterboxAnimTimer_ += FpsCounter::deltaTime;
        float t = std::clamp(letterboxAnimTimer_ / letterboxAnimDuration_, 0.0f, 1.0f);
        float eased = EaseOutCubic(0.0f, 1.0f, t);
        float h = letterboxStartHeight_ + (letterboxEndHeight_ - letterboxStartHeight_) * eased;
        letterbox_->SetBoxHeight(h);
        letterbox_->Update();
    }

	//============================
	// FOV設定
	//============================
	if (!bossIntroPlaying_ && !bossOutroPlaying_) {
		float desiredFov = 0.7f; // 通常
		if (player_->IsRushing()) {
			desiredFov = 1.0f; // 突進中
		}/* else if (player_->IsCharging() || player_->IsPreRushing()) {
			// 突進レベルに応じてFOVを変化させる
			int rushLevel = player_->GetRushChargeLevel();
			desiredFov = 0.4f + static_cast<float>(3 - rushLevel) * 0.1f;
		}*/
		cameraController_->SetDesiredFov(desiredFov);

		// ジャンプ中は見下ろし視点オフセットを適用
		if (player_->IsJump()) {
			// 斜め上から俯瞰するように、カメラ位置を上方向に、注視点を少し下へ
			Vector3 eyeOffset{ 0.0f, 16.0f, 0.0f };
			Vector3 lookOffset{ 0.0f, -4.0f, 0.0f };
			cameraController_->SetViewOffset(eyeOffset, lookOffset, 0.2f);
			cameraController_->EnableViewOffset(true);
		} else if (isBossLockOn_) {
			// ボスロックオン時は通常より低めのカメラアングルにする
			Vector3 eyeOffset{ 0.0f, -8.0f, 0.0f };
			Vector3 lookOffset{ 0.0f, 0.0f, 0.0f };
			cameraController_->SetViewOffset(eyeOffset, lookOffset, 0.2f);
			cameraController_->EnableViewOffset(true);
		} else {
			cameraController_->EnableViewOffset(false);
		}
	}

	if (isTitleLocked_ || bossIntroPlaying_ || bossOutroPlaying_) {
		cameraController_->Update(nullptr, nullptr);
	} else {
		cameraController_->Update(context_->inputCommand, context_->input);
	}
	mainCamera_->SetCamera(cameraController_->GetCamera());

	//========================================
	// 敵の更新処理
	//========================================
#pragma region EnemyUpdate
	// 敵の移動処理
	bossEnemy_->Update(player_->GetPlayerPos());
	if (!isTitleLocked_) {
		enemyAttackManager_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());
		enemyAttackManager_->SetPlayerHp(player_->GetCurrentHP());
	}
	// 敵の影の更新処理
	bossEnemyShadow_->Update();
	// 突進攻撃演出の更新処理
	if (bossEnemy_->IsRushAttack()) {
		enemyRushEffect_->SetActiveParticle(true, bossEnemy_->GetWorldPosition(), bossEnemy_->GetRushVelocity());
	} else {
		enemyRushEffect_->SetActiveParticle(false, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,1.0f });
	}
	enemyRushEffect_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// 風攻撃演出の更新処理
	if (enemyAttackManager_->IsWind()) {
		enemyWindAttackParticle_->SetIsLoop(true);
		enemyWindAttackParticle_->SetEmitterPos(Vector3(bossEnemy_->GetWorldPosition().x, bossEnemy_->GetWorldPosition().y + 2.0f, bossEnemy_->GetWorldPosition().z));
		enemyWindAttackParticle_->SetVelocity(enemyAttackManager_->GetWindVelocity());
		enemyWindShadowAttackParticle_->SetIsLoop(true);
		enemyWindShadowAttackParticle_->SetEmitterPos(Vector3(bossEnemy_->GetWorldPosition().x, bossEnemy_->GetWorldPosition().y + 2.0f, bossEnemy_->GetWorldPosition().z));
		enemyWindShadowAttackParticle_->SetVelocity(enemyAttackManager_->GetWindVelocity());
	} else {
		enemyWindAttackParticle_->SetIsLoop(false);
		enemyWindShadowAttackParticle_->SetIsLoop(false);
	}
	enemyWindAttackParticle_->Update();
	enemyWindShadowAttackParticle_->Update();

	// ボスの翼の演出
	enemyWingsParticleParticle_->SetIsLoop(bossEnemy_->IsWingsEffect());
	enemyWingsParticleParticle_->SetEmitterPos(
		Vector3(bossEnemy_->GetWorldPosition().x, bossEnemy_->GetWorldPosition().y + 3.0f, bossEnemy_->GetWorldPosition().z));
	enemyWingsParticleParticle_->Update();

	// ボスの入りの翼演出
	enterWingsParticle_->SetIsLoop(bossEnemy_->IsEnterWingsEffect());
	enterWingsParticle_->SetEmitterPos(bossEnemy_->GetWorldPosition() + Vector3(0.0f, 3.0f, 0.0f));
	enterWingsParticle_->Update();
	enterBurstWingsParticle_->SetIsLoop(bossEnemy_->IsEnterBurstWingsEffect());
	enterBurstWingsParticle_->SetEmitterPos(bossEnemy_->GetWorldPosition() + Vector3(0.0f, 4.0f, 0.0f));
	enterBurstWingsParticle_->Update();

	// 敵が死亡した時にbgmを止める
	if (bossEnemy_->IsStopBGm()) {
		AudioManager::GetInstance().Stop(titleBGMHandle_);
		AudioManager::GetInstance().Stop(gameBGMHandle_);
	}

	// ボスがヒットした時の演出
	if (bossEnemy_->IsHit()) {
		bossEnemyModel_->SetDefaultColor({ 1.0f,1.0f,1.0f,bossEnemy_->GetAlpha() });
	}

	// プレイヤーの攻撃状態を取得
	bossEnemy_->SetIsPlayerDownAttack(player_->IsAttackDown());
	
	// プレイヤーがダメージ無敵中の点滅演出
	if (player_ && player_->IsDamagedInvincible()) {
		float a = player_->GetDamageFlashAlpha();
		playerModel_->SetDefaultColor({ 1.0f,1.0f,1.0f, a });
	} else {
		// 通常は不透明
		playerModel_->SetDefaultColor({ 1.0f,1.0f,1.0f, 1.0f });
	}

	// 撃破演出
	if (bossEnemy_->IsDestroyEffect()) {
		enemyAttackManager_->AddEnemyDestroyEffect(bossEnemy_->GetWorldPosition());
	}
#pragma endregion

	// StageWallPlaneの更新
#ifdef _DEBUG
	StageWallPlane::ApplyDebugParam(stageWallPlaneMaterial_.get());
#endif
	for (auto &plane : stageWallPlanes_) {
		plane.Update();
	}
	// ボスが常に纏っているパーティクル
	bossWearParticle_->SetEmitterPos(bossEnemy_->GetWorldPosition());
	bossWearParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());
	bossWearAdditionParticle_->SetEmitterPos(bossEnemy_->GetWorldPosition());
	bossWearAdditionParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// 当たり判定の更新処理
	UpdateCollision();

	// ボスのHpUIの更新処理
	bossHpUI_->SetCurrentHp(bossEnemy_->GetCurrentHP());
	bossHpUI_->Update();

	// プレイヤーのHpUIの更新処理
	playerHpUI_->SetCurrentHp(player_->GetCurrentHP());
	playerHpUI_->Update();

	// GameOver判定: プレイヤーが生存していなければGameOverUIを有効化
	if (player_ && !player_->IsAlive()) {
		if (gameOverUI_) {
			gameOverUI_->SetActive(true);
		}
	}

	// GameOverUIの更新処理
	gameOverUI_->Update();
	if (gameOverUI_->IsActive()) {
        if (context_->inputCommand->IsCommandActive("Up") ||
			context_->inputCommand->IsCommandActive("Down")) {
			AudioManager::GetInstance().Play(selectSEHandle_, 0.5f, false);
        }
		if (context_->inputCommand->IsCommandActive("Start")) {
			AudioManager::GetInstance().Play(decideSEHandle_, 0.5f, false);
		}
    }

	// ステージの更新処理
	stageManager_->Update(player_->GetPlayerPos(),player_->GetDir());

	// 空気を演出するためのパーティクル
	airParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// ボス撃破時のフェード処理
	if (bossEnemy_->GetCurrentHP() <= 0) {
		if (!isBossDestroyFade_) {
			bossDestroyFade_->SetActive();
			isBossDestroyFade_ = true;
			bossOutroScheduled_ = true;
		}
	}
	// フェードが有効中の時、中間に来ていればリセットする
	if (bossDestroyFade_->IsActive()) {
		if (bossDestroyFade_->IsMiddle()) {
			bossEnemy_->SetResetPosition();
		}
	}
	bossDestroyFade_->Update();

	// クリア処理
	clearUI_->Update();

	// 矢印UI
	if (bossEnemy_->Isnow() && !isTitleLocked_) {
		arrowUI_->SetIsActive(true);
	} else {
		arrowUI_->SetIsActive(false);
	}
	
	if (arrowUI_->IsActive()) {
		arrowUI_->Update(mainCamera_->GetWorldMatrix());
	}

#ifdef _DEBUG
	// 地面マテリアルの更新処理
	terrain_->Update();
	bgRock_->Update();
#endif

	// TutorialUI の更新（描画は後段。ボス登場演出中は更新のみ）
	if (tutorialUI_) {
		for (auto &spr : tutorialUI_->GetSprites()) { spr->Update(); }
	}

    // BossAppearanceUI 更新
    if (bossAppearanceUI_ && bossAppearanceUI_->IsActive()) {
        bossAppearanceUI_->Update();
    }
}

void TDGameScene::Draw(const bool &isDebugView) {

	// 描画に使用するカメラを設定
	if (isDebugView) {
		// 描画に使用するカメラを設定
		ModelRenderer::SetCamera(context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
	} else {
		// 描画に使用するカメラを設定
		ModelRenderer::SetCamera(mainCamera_->GetVPMatrix(), mainCamera_->GetCameraResource());
	}

	if (isDebugView) {
		CustomRenderer::SetCamera(context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
	} else {
		CustomRenderer::SetCamera(mainCamera_->GetVPMatrix(), mainCamera_->GetCameraResource());
	}

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// 天球の描画
	ModelRenderer::Draw(skyDomeModel_, skyDomeWorldTransform_);

	// 背景のオブジェクトを描画
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(bgRockModel_, bgRock_->GetWorldTransform());
	
	if (bossEnemy_->GetBossState() == BossState::Egg) {
		ModelRenderer::Draw(bossEggModel_, bossEnemy_->GetWorldTransform());
	}

	// 岩を描画
	CustomRenderer::PreDraw(CustomRenderMode::Rock);
	CustomRenderer::DrawRock(bgIceRockModel_, bgRock_->GetWorldTransform(), sceneLightingController_->GetResource(), bgRock_->GetMaterial());

	// アニメーションの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::AnimationModel);

	if (!bossIntroPlaying_ && !bossOutroPlaying_) {
		// プレイヤーのアニメーションを描画
		ModelRenderer::DrawAnimationWithLight(playerModel_, player_->GetWorldTransform(), sceneLightingController_->GetResource());
		// プレイヤーの影を描画する
		ModelRenderer::DrawAnimation(playerModel_, playerShadow_->GetWorldTransform(), &playerShadow_->GetMaterial());
	}

	// 敵の影を描画する
	if (bossEnemy_->GetBossState() != BossState::Egg) {
		ModelRenderer::DrawAnimation(bossEnemyModel_, bossEnemyShadow_->GetWorldTransform(), &bossEnemyShadow_->GetMaterial());
	}

	// ボスのアニメーション専用shader
	CustomRenderer::PreDraw(CustomRenderMode::BossAnimation);

	// 敵を描画
    if (bossEnemy_->GetBossState() != BossState::Egg) {
		//ModelRenderer::DrawAnimationWithLight(bossEnemyModel_, bossEnemy_->GetWorldTransform(), sceneLightingController_->GetResource());
		CustomRenderer::DrawAnimationWithLight(bossEnemyModel_, bossEnemy_->GetWorldTransform(), sceneLightingController_->GetResource(), bossEnemy_->GetMaterial());
	}

	// 氷のテスト描画
	CustomRenderer::PreDraw(CustomRenderMode::Ice);
	CustomRenderer::DrawIce(icePlaneModel_, terrain_->GetWorldTransform(), sceneLightingController_->GetResource(), terrain_->GetMaterial());
	/*for (auto &plane : stageWallPlanes_) {
		if (plane.GetIsAlive()) {
			CustomRenderer::DrawIce(icePlaneModel_, plane.GetWorldTransform(), sceneLightingController_->GetResource(), stageWallPlaneMaterial_.get());
		}
	}*/

	// 通常モデルの描画前処理
	//ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// 回復用のハート描画
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);
	// 回復用ハートを描画する
	for (auto& heart : enemyAttackManager_->GetHeartList()) {
		if (heart->IsAlive()) {
			ModelRenderer::Draw(heartModel_, heart->GetWorldTransform(), &heart->GetMaterial());
		}
	}
	
	// 氷岩の両面描画
	CustomRenderer::PreDraw(CustomRenderMode::RockBoth);

	const std::list<std::unique_ptr<IceFall>>& iceFalls = enemyAttackManager_->GetIceFalls();
	for (auto& iceFall : iceFalls) {
		if (iceFall->IsAlive()) {
			//ModelRenderer::DrawLight(sceneLightingController_->GetResource());
			CustomRenderer::DrawRock(iceFallModel_, iceFall->GetWorldTransform(), sceneLightingController_->GetResource(), iceFall->GetMaterial());
			//ModelRenderer::Draw(iceFallModel_, iceFall->GetShadowWorldTransform(), &iceFall->GetShadowMaterial());
		}
	}

	// 氷柱の破壊演出を描画
	for (const std::unique_ptr<BreakIceFallParticle>& iceFall : enemyAttackManager_->GetBreakIceFallParticles()) {
		if (!iceFall->IsFinished()) {
			for (auto& particle : iceFall->GetParticleDatas()) {
				CustomRenderer::DrawRock(breakIceFallModel_, particle.worldTransform, sceneLightingController_->GetResource(), particle.material.get());
			}
		}
	}

	// 撃破演出で使用する氷の塊
	for (const std::unique_ptr<EnemyDestroyEffect>& effect : enemyAttackManager_->GetEnemyDestroyEffect()) {
		if (!effect->IsFinished()) {
			for (auto& particle : effect->GetParticleDatas()) {
				CustomRenderer::DrawRock(breakIceFallModel_, particle.worldTransform, sceneLightingController_->GetResource(), particle.material.get());
			}
		}
	}

	// ステージ壁を描画する
	stageManager_->Draw(wallModel_, sceneLightingController_->GetResource());

	// 3Dモデルの両面描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModelBoth);

	// プレイヤーのエフェクト描画
	if (player_->IsCharging() || player_->IsPreRushing()) {
		int active = playerChargeEffect_->GetActiveCount();
		auto &wts = playerChargeEffect_->GetWorldTransforms();
		for (int i = active - 1; i >= 0; --i) {
			ModelRenderer::Draw(playerChargeEffectModel_, wts[i], playerChargeEffect_->GetMaterial());
		}
	}
    if (player_->IsRushing()) {
		for (auto &rushEffect : playerRushEffect_->GetWorldTransforms()) {
			ModelRenderer::Draw(playerChargeEffectModel_, rushEffect);
		}
    }
	if (player_->IsAttackDown()) {
		for (auto &attackDownEffect : playerAttackDownEffect_->GetWorldTransforms()) {
			ModelRenderer::Draw(playerAttackDownEffectModel_, attackDownEffect, playerAttackDownEffect_->GetMaterial());
		}
	}
	if (playerLandingEffect_ && playerLandingEffect_->IsActive()) {
		auto &wts = playerLandingEffect_->GetWorldTransforms();
		auto &mats = playerLandingEffect_->GetMaterials();
		for (size_t i = 0; i < wts.size(); ++i) {
			ModelRenderer::Draw(playerLandingEffectModel_, wts[i], mats[i].get());
		}
	}

	// ボスの突進攻撃の描画
	if (bossEnemy_->IsRushAttack()) {
		for (auto &rushEffect : enemyRushEffect_->GetWorldTransforms()) {
			ModelRenderer::Draw(enemyRushModel_, rushEffect);
		}
	}

	// ボスの撃破演出
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);
	for (const std::unique_ptr<EnemyDestroyEffect>& effect : enemyAttackManager_->GetEnemyDestroyEffect()) {
		if (!effect->IsFinished()) {
			for (auto& world : effect->breakEffect_->GetWorldTransforms()) {
				ModelRenderer::Draw(planeModel_, world, &effect->GetMaterial());
			}
		}
	}

	if (arrowUI_->IsActive()) {
		ModelRenderer::Draw(planeModel_, *arrowUI_->GetWorldTransform(), &arrowUI_->GetMaterial());
	}

	// プレイヤーの攻撃演出を描画
	ModelRenderer::PreDraw(RenderMode3D::DefaultModelAdd);
	if (playerAttackEffect_->IsActive()) {
		ModelRenderer::Draw(planeModel_, playerAttackEffect_->GetWorldTransforms(), &playerAttackEffect_->GetMaterial());

		if (playerAttackEffect_->IsSubDraw()) {
			ModelRenderer::Draw(planeModel_, playerAttackEffect_->GetSubWorldTransforms(), &playerAttackEffect_->GetSubMaterial());
		}
	}

	// インスタンシング描画前処理
	ModelRenderer::PreDraw(RenderMode3D::InstancingBothNone);
	// ボスの風攻撃を描画
	ModelRenderer::DrawInstancing(windModel_, enemyWindShadowAttackParticle_->GetCurrentNumInstance(), *enemyWindShadowAttackParticle_->GetWorldTransforms());

	// インスタンシング描画前処理
	ModelRenderer::PreDraw(RenderMode3D::InstancingBoth);
	// ボスの風攻撃を描画
	ModelRenderer::DrawInstancing(windModel_, enemyWindAttackParticle_->GetCurrentNumInstance(), *enemyWindAttackParticle_->GetWorldTransforms());

	// ボスの翼の演出を描画
	ModelRenderer::DrawInstancing(wingModel_, enemyWingsParticleParticle_->GetCurrentNumInstance(), *enemyWingsParticleParticle_->GetWorldTransforms());

	// ボスの入りの翼の演出を描画
	ModelRenderer::DrawInstancing(wingModel_, enterWingsParticle_->GetCurrentNumInstance(), *enterWingsParticle_->GetWorldTransforms());
	ModelRenderer::DrawInstancing(wingModel_, enterBurstWingsParticle_->GetCurrentNumInstance(), *enterBurstWingsParticle_->GetWorldTransforms());

	// インスタンシング描画前処理
	ModelRenderer::PreDraw(RenderMode3D::Instancing);

	// 壁破壊のパーティクルを描画
	for (auto& particle : stageManager_->GetBreakWallParticles()) {
		ModelRenderer::DrawInstancing(wallModel_, particle->GetCurrentNumInstance(), *particle->GetWorldTransforms());
	}

	// 壁の破壊表現
	for (auto& particle : stageManager_->GetPlayerBreakWallParticle()) {
		ModelRenderer::DrawInstancing(wallModel_, particle->GetCurrentNumInstance(), *particle->GetWorldTransforms());
	}

	// 複数モデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::InstancingAdd);

	// 氷柱を落とすまでの演出を描画
	for (auto &iceFallEffect : enemyAttackManager_->GetIceFallEffectDatas()) {
		if (!iceFallEffect.isActive) { continue; }
		ModelRenderer::DrawInstancing(planeModel_, iceFallEffect.particle->GetCurrentNumInstance(), *iceFallEffect.particle->GetWorldTransforms());
	}

	// 敵の撃破時の演出
	for (const std::unique_ptr<EnemyDestroyEffect>& effect : enemyAttackManager_->GetEnemyDestroyEffect()) {
		if (!effect->IsFinished()) {
			auto& particle = effect->GetSmallParticle();
			ModelRenderer::DrawInstancing(planeModel_, particle->GetCurrentNumInstance(), *particle->GetWorldTransforms());
		}
	}

	// プレイヤーの攻撃演出
	ModelRenderer::DrawInstancing(planeModel_, playerAttackEffect_->smallParticle_->GetCurrentNumInstance(), *playerAttackEffect_->smallParticle_->GetWorldTransforms());

	// 突進する時の風パーティクル
	ModelRenderer::DrawInstancing(planeModel_, enemyRushEffect_->enemyRushParticle_->GetCurrentNumInstance(), *enemyRushEffect_->enemyRushParticle_->GetWorldTransforms());

	// ボスの纏っているパーティクルを描画
	ModelRenderer::DrawInstancing(planeModel_, bossWearAdditionParticle_->GetCurrentNumInstance(), *bossWearAdditionParticle_->GetWorldTransforms());
	ModelRenderer::DrawInstancing(planeModel_, bossWearParticle_->GetCurrentNumInstance(), *bossWearParticle_->GetWorldTransforms());

	// 空気を演出するためのパーティクルを描画
	ModelRenderer::DrawInstancing(planeModel_, airParticle_->GetCurrentNumInstance(), *airParticle_->GetWorldTransforms());

	// プレイヤーの回復演出
	ModelRenderer::DrawInstancing(planeModel_, playerGetHeartParticle_->GetCurrentNumInstance(), *playerGetHeartParticle_->GetWorldTransforms());

	// プレイヤーの歩く演出
	ModelRenderer::DrawInstancing(planeModel_, playerWalkParticle_->GetCurrentNumInstance(), *playerWalkParticle_->GetWorldTransforms());

#ifdef _DEBUG

	// デバック描画
	debugRenderer_->DrawAll(isDebugView ? context_->debugCamera_->GetVPMatrix() : mainCamera_->GetVPMatrix());
#endif
}

void TDGameScene::DrawUI() {

	//======================================================
	// 2D描画
	//======================================================

	SpriteRenderer::PreDraw(RenderMode2D::Normal);

    if (letterbox_) {
        if (auto s = letterbox_->GetTopSprite()) { SpriteRenderer::Draw(s, 0); }
        if (auto s = letterbox_->GetBottomSprite()) { SpriteRenderer::Draw(s, 0); }
    }

    // タイトルスプライト
    if (titleSprite_) { SpriteRenderer::Draw(titleSprite_.get(), titleGH_); }
    if (spaceSprite_) { SpriteRenderer::Draw(spaceSprite_.get(), spaceGH_); }

    // TutorialUI はボス登場演出が始まって以降は描画しない
    if (tutorialUI_ && !bossIntroPlaying_ && !uiDisabledForIntro_ && !bossIntroFinished_) {
        const auto &sprites = tutorialUI_->GetSprites();
        const auto &handles = tutorialUI_->GetTextureHandles();
        size_t n = std::min(sprites.size(), handles.size());
        for (size_t i = 0; i < n; ++i) {
            SpriteRenderer::Draw(sprites[i].get(), handles[i]);
        }
    }

	// プレイヤーHP
    if (!bossIntroPlaying_ && !isBossDestroyFade_) {
        for (const auto &sprite : playerHpUI_->GetHpSprites()) {
            SpriteRenderer::Draw(sprite.get(), playerHpUI_->GetHpIconGH());
        }
    }

	// ボスのHPUIは演出が終わったあとにのみ描画
    if (bossIntroFinished_ && !isBossDestroyFade_) {
        SpriteRenderer::Draw(bossHpUI_->GetFrameSprite(), 0);
        SpriteRenderer::Draw(bossHpUI_->GetEffectSprite(), 0);
        SpriteRenderer::Draw(bossHpUI_->GetSprite(), 0);
        SpriteRenderer::Draw(bossHpUI_->GetNameSprite(), bossNameGH_);
    }

    // 操作ガイド
    if (!bossIntroPlaying_ && playGuideSprite_ && !isBossDestroyFade_) {
        SpriteRenderer::Draw(playGuideSprite_.get(), playGuideGH_);
    }

    // GameOverUI
    if (gameOverUI_->IsActive()) {
        SpriteRenderer::Draw(gameOverUI_->GetBgSprite(), gameOverUI_->GetBgGH());
        SpriteRenderer::Draw(gameOverUI_->GetLogoSprite(), gameOverUI_->GetLogoGH());
        SpriteRenderer::Draw(gameOverUI_->GetRetrySprite(), gameOverUI_->GetRetryGH());
        SpriteRenderer::Draw(gameOverUI_->GetTitleSprite(), gameOverUI_->GetTitleGH());
    }

    // クリアUI
    if (clearUI_->IsActive()) {
        SpriteRenderer::Draw(clearUI_->GetBgSprite(), 0);
        SpriteRenderer::Draw(clearUI_->GetClearBackSprite(), 0);
        SpriteRenderer::Draw(clearUI_->GetClearSprite(), clearUI_->GetClearTexture());
        SpriteRenderer::Draw(clearUI_->GetGuideSprite(), clearUI_->GetGuidTexture());
		SpriteRenderer::Draw(clearUI_->GetClearTimeTextSprite(), clearUI_->GetClearTImeTextTexture());
		SpriteRenderer::Draw(clearUI_->GetnumDottoSprite(), clearUI_->GetDottoTexture());

		for (auto& num : clearUI_->GetNumberSprite()) {
			SpriteRenderer::Draw(num.numSprite_.get(), num.number);
		}
    }

    // 撃破フェード
    if (bossDestroyFade_->IsActive()) {
        SpriteRenderer::Draw(bossDestroyFade_->GetSprite(), 0);
    }

    // BossAppearanceUI の描画
    if (bossAppearanceUI_ && bossAppearanceUI_->IsActive()) {
        SpriteRenderer::Draw(bossAppearanceUI_->GetBossTitleSprite(), bossAppearanceUI_->GetCurrentBossTitleGH());
        SpriteRenderer::Draw(bossAppearanceUI_->GetBossNameSprite(), bossAppearanceUI_->GetCurrentBossNameGH());
        SpriteRenderer::Draw(bossAppearanceUI_->GetBossAppearanceStarSprite(), bossAppearanceUI_->GetBossAppearanceStarGH());
    }

    // ダメージ数字の描画
    if (playerDamageNumberEffect_ && playerDamageNumberEffect_->IsActive()) {
        const auto &spritesRef = playerDamageNumberEffect_->GetDigitSprites();
        const auto &handlesRef = playerDamageNumberEffect_->GetTextureHandles();
        size_t n = std::min(spritesRef.size(), handlesRef.size());
        for (size_t i = 0; i < n; ++i) {
            SpriteRenderer::Draw(spritesRef[i], handlesRef[i]);
        }
    }
}
 
 void TDGameScene::InputRegisterCommand() {
	// 移動の入力コマンドを登録する
	context_->inputCommand->RegisterCommand("MoveUp", { {InputState::KeyPush, DIK_W },{InputState::PadLeftStick,0,{0.0f,1.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_UP } });
	context_->inputCommand->RegisterCommand("MoveDown", { {InputState::KeyPush, DIK_S },{InputState::PadLeftStick,0,{0.0f,-1.0f},0.2f}, {InputState::PadPush, XINPUT_GAMEPAD_DPAD_DOWN} });
	context_->inputCommand->RegisterCommand("MoveLeft", { {InputState::KeyPush, DIK_A },{InputState::PadLeftStick,0,{-1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_LEFT } });
	context_->inputCommand->RegisterCommand("MoveRight", { {InputState::KeyPush, DIK_D },{InputState::PadLeftStick,0,{1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_RIGHT } });
	// ジャンプコマンドを登録する
	context_->inputCommand->RegisterCommand("Jump", { {InputState::KeyTrigger, DIK_SPACE},{InputState::PadTrigger, XINPUT_GAMEPAD_A} });

    // AttackDownコマンド
	context_->inputCommand->RegisterCommand("AttackDown", { {InputState::MouseTrigger, 0}, {InputState::PadTrigger, XINPUT_GAMEPAD_X} });
    // RushChargeコマンド
    context_->inputCommand->RegisterCommand("RushCharge", { {InputState::MousePush, 0}, {InputState::PadPush, XINPUT_GAMEPAD_X} });
    // RushStartコマンド
    context_->inputCommand->RegisterCommand("RushStart", { {InputState::MouseRelease, 0}, {InputState::PadRelease, XINPUT_GAMEPAD_X} });

	// トリガーのテスト
	context_->inputCommand->RegisterCommand("TestTrigger", { {InputState::PadLeftTriggerTrigger,0,{0.0f,0.0f},0.2f} });

	// ロックオンコマンド
    context_->inputCommand->RegisterCommand("LockOnBoss", { {InputState::KeyTrigger, DIK_TAB }, {InputState::PadRightTriggerTrigger,0,{0.0f,0.0f},0.2f} });
	
	// カメラ操作のコマンドを登録する
	context_->inputCommand->RegisterCommand("CameraMoveLeft", { { InputState::KeyPush, DIK_LEFT },{InputState::PadRightStick,0,{-1.0f,0.0f},0.2f} });
	context_->inputCommand->RegisterCommand("CameraMoveRight", { { InputState::KeyPush, DIK_RIGHT },{InputState::PadRightStick,0,{1.0f,0.0f},0.2f} });

	// Startゲーム開始（EnterキーまたはAボタン）
    context_->inputCommand->RegisterCommand("Start", { {InputState::KeyTrigger, DIK_RETURN}, {InputState::PadTrigger, XINPUT_GAMEPAD_A},{InputState::KeyTrigger, DIK_SPACE}, {InputState::MouseTrigger, 0} });
	// メニュー移動
	context_->inputCommand->RegisterCommand("Up", { {InputState::KeyTrigger, DIK_W},{InputState::KeyTrigger, DIK_UP}, {InputState::PadTrigger, XINPUT_GAMEPAD_DPAD_UP} });
	context_->inputCommand->RegisterCommand("Down", { {InputState::KeyTrigger, DIK_S},{InputState::KeyTrigger, DIK_DOWN}, {InputState::PadTrigger, XINPUT_GAMEPAD_DPAD_DOWN} });
	// バック/終了
	context_->inputCommand->RegisterCommand("Back", { {InputState::KeyTrigger, DIK_ESCAPE}, {InputState::PadTrigger, XINPUT_GAMEPAD_B} });
}

void TDGameScene::UpdateCollision() {
    collisionManager_->AddCollider(player_->GetCollider());
#ifdef _DEBUG
    debugRenderer_->AddSphere(player_->GetSphereData());
#endif

#ifdef _DEBUG
    debugRenderer_->AddSphere(player_->GetSphereData());
#endif

	//=================================
	// 敵の当たり判定
	//=================================
#pragma region EnemyCollider
	// 敵の当たり判定を登録する
	if (bossEnemy_->GetCurrentHP() > 0) {
		collisionManager_->AddCollider(bossEnemy_->GetCollider());
	}
#ifdef _DEBUG
	debugRenderer_->AddSphere(bossEnemy_->GetSphereData(),{1.0f,1.0f,0.0f,1.0f});
#endif

	// 氷柱の当たり判定を登録する
	const std::list<std::unique_ptr<IceFall>>& iceFalls = enemyAttackManager_->GetIceFalls();
	for (auto& iceFall : iceFalls) {
		if (iceFall->IsAlive()) {
			collisionManager_->AddCollider(iceFall->GetCollider());
#ifdef _DEBUG
			debugRenderer_->AddSphere(iceFall->GetSphereData(), { 1.0f,1.0f,0.0f,1.0f });
#endif
		}
	}

	// 敵の風攻撃の当たり判定を登録する
	if (enemyAttackManager_->IsWind()) {
		for (auto& collider : enemyAttackManager_->GetWindColliders()) {
			collisionManager_->AddCollider(collider.get());
#ifdef _DEBUG
			debugRenderer_->AddSphere({ collider->GetWorldPosition(),collider->GetRadius()}, {1.0f,1.0f,0.0f,1.0f});
#endif
		}
	}

	// 回復ハート
	for (auto& heart : enemyAttackManager_->GetHeartList()) {
		if (!heart->GetIsStop()) {
			collisionManager_->AddCollider(heart->GetCollider());
#ifdef _DEBUG
			debugRenderer_->AddSphere({ heart->GetCollider()->GetWorldPosition(),heart->GetCollider()->GetRadius() }, { 1.0f,1.0f,0.0f,1.0f });
#endif
		}
	}

#pragma endregion

	//=====================================
	// オブジェクトの当たり判定
	//=====================================

    // 床の当たり判定を登録する
    collisionManager_->AddCollider(terrain_->GetCollider());

	// 生存している壁の要素を取得する
	const std::vector<Wall*> aliveWalls = stageManager_->GetAliveWalls();
	for (auto& wall : aliveWalls) {
		// 当たり判定を追加
		collisionManager_->AddCollider(wall->GetCollider());
#ifdef _DEBUG
		// デバック描画に追加
		debugRenderer_->AddBox(wall->GetOBBData());
#endif
	}

	// 衝突判定
	collisionManager_->CheckAllCollisions();
}

void TDGameScene::DebugUpdate() {
#ifdef _DEBUG

	// ステージ作成のデバック用
	stageManager_->DebugUpdate();

	// デバックリストを削除
	debugRenderer_->Clear();
	// 生存している壁の要素を取得する
	const std::vector<Wall*> aliveWalls = stageManager_->GetAliveWalls();
	for (auto& wall : aliveWalls) {
		// デバック描画に追加
		debugRenderer_->AddBox(wall->GetOBBData());
	}

	// 地面マテリアルの更新処理
	terrain_->Update();

	// 背景の岩オブジェクトの更新処理
	bgRock_->Update();

	// 当たり判定の表示管理
	ImGui::Begin("DebugCollision");
	ImGui::Checkbox("IsDrawCollision", &isDrawCollision_);
	debugRenderer_->SetEnabled(isDrawCollision_);
	ImGui::End();
#endif
}

void TDGameScene::UpdateStartAnimation() {
	cameraController_->Update(context_->inputCommand, context_->input);
	mainCamera_->SetCamera(cameraController_->GetCamera());
}