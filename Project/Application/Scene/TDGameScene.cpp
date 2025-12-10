#include"TDGameScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"SpriteRenderer.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
#include"LogManager.h"
#include"CollisionConfig.h"
#include<numbers>

#include"Application/CollisionTypeID.h"
#include"Extension/CustomRenderer.h"
#include"FPSCounter.h"

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
	stageManager_->Update();

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
		cameraController_->StartCameraShake(5.0f, 1.0f, 100.0f,
			[](const Vector3 &a, const Vector3 &b, float t) { return EaseInOutCubic(a, b, t); },
			CameraController::ShakeOrigin::TargetPosition,
            true, false, true, false);
        if (playerLandingEffect_) {
            playerLandingEffect_->Emitter(player_->GetWorldTransform().GetWorldPosition());
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

	// 敵のアニメーションデータを取得する
	enemyAnimationData_[static_cast<size_t>(enemyAnimationType::BaseMove)] = context_->animationManager->GetNameByAnimations("BossBirdBaseMove");
	enemyAnimationData_[static_cast<size_t>(enemyAnimationType::IceBreath)] = context_->animationManager->GetNameByAnimations("BossBird.IceBreath");
	enemyAnimationData_[static_cast<size_t>(enemyAnimationType::Rush)] = context_->animationManager->GetNameByAnimations("BossBirdRush");
	enemyAnimationData_[static_cast<size_t>(enemyAnimationType::Scream)] = context_->animationManager->GetNameByAnimations("BossBirdScream");

	// ボスのアニメーションの再生を管理する
	bossEnemyAnimator_ = std::make_unique<Animator>();
	bossEnemyAnimator_->Initialize(bossEnemyModel_, &enemyAnimationData_[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);

	// ボス敵クラスを初期化
	bossEnemy_ = std::make_unique<BossEnemy>();
	bossEnemy_->Initialize(stageManager_->GetRadius(), enemyAttackManager_.get(), bossEnemyAnimator_.get(), 
		&enemyAnimationData_,debugRenderer_.get());
	
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
		// 初回実行フラグを解除
		TDGameScene::SetIsFirstGameStart(false);
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
    playerHpUI_->Initialize(player_->GetMaxHP());

	// ゲームオーバーUIの初期化
	gameOverUI_ = std::make_unique<GameOverUI>();
	gameOverUI_->Initialize(context_->input, context_->inputCommand, context_->textureManager);
	// リトライ・タイトルのコールバックを設定
	gameOverUI_->SetOnRetryClicked([this]() {
		// リトライが選択されたら現在のシーンかResultへ遷移
		if (bossEnemy_ && bossEnemy_->GetCurrentHP() == 0) {
			nextSceneState_ = SceneState::Result;
		} else {
			nextSceneState_ = SceneState::TDGame;
		}
		isFinished_ = true;
	});
	gameOverUI_->SetOnTitleClicked([this]() {
		nextSceneState_ = SceneState::Title;
		isFinished_ = true;
	});

	// 空気を演出するためのパーティクル
	airParticle_ = std::make_unique<ParticleBehavior>();
	airParticle_->Initialize("AirParticle", 128);
	airParticle_->Emit({ 0.0f,0.0f,0.0f });

	transitionStartTarget_ = Vector3{ 0.0f,32.0f,0.0f };
	transitionEndTarget_ = player_ ? player_->GetWorldTransform().GetWorldPosition() : transitionStartTarget_;

	// cameraController のターゲットを初期注視点に設定して内部状態を同期
	cameraController_->SetTarget(transitionStartTarget_);
	cameraController_->SetCameraCoordinateType(CameraController::CameraCoodinateType::Spherical);
	cameraController_->SetDesiredFov(0.7f);
	cameraController_->SetDesiredAsCurrent();
	cameraController_->Update(context_->inputCommand, context_->input);
	cameraController_->SetCurrentAsDesired();
	cameraController_->Update(context_->inputCommand, context_->input);
	mainCamera_->SetCamera(cameraController_->GetCamera());
	mainCamera_->Update();

	isTitleLocked_ = true;
	isTransitioning_ = false;
	transitionTimer_ = 0.0f;

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

	// クリアUI
	clearUI_ = std::make_unique<ClearUI>();
	clearUI_->Initialize(context_->inputCommand, context_->textureManager);

	// ボスの撃破時のフェード
	bossDestroyFade_ = std::make_unique<BossDestroyFade>();
	bossDestroyFade_->Initialize();
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

			// スプライトのフェード
			float alpha = 1.0f - eased;
			if (titleSprite_) titleSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, alpha));
			if (spaceSprite_) spaceSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, alpha));

			if (transitionTimer_ >= kTransitionDuration_) {
				isTransitioning_ = false;
				isTitleLocked_ = false;
			}
		}

		// カメラ更新
		cameraController_->Update(context_->inputCommand, context_->input);
		mainCamera_->SetCamera(cameraController_->GetCamera());
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
		if (cameraController_) {
			cameraController_->SetDesiredFov(1.0f);
			cameraController_->StartCameraShake(64.0f, kBossHitFreezeDuration, 256.0f,
				[](const Vector3 &a, const Vector3 &b, float t) { return EaseInOutCubic(a, b, t); },
				CameraController::ShakeOrigin::TargetAndCameraPosition,
				true, false, true, false);
		}
		// 攻撃演出のエミッター
		playerAttackEffect_->Emitter(player_->GetPlayerPos());
	}
	prevBossHit_ = currentBossHit;

	if (isBossHitFreezeActive_) {
		bossHitFreezeTimer_ -= GameEngine::FpsCounter::deltaTime;
		cameraController_->Update(context_->inputCommand, context_->input);
		mainCamera_->SetCamera(cameraController_->GetCamera());

		if (bossHitFreezeTimer_ <= 0.0f) {
			isBossHitFreezeActive_ = false;
			bossHitFreezeTimer_ = 0.0f;
		}

		return;
	}

	// デバックリストを削除
	debugRenderer_->Clear();

	// 当たり判定をリセット
	collisionManager_->ClearList();

	// ライトの更新処理
	sceneLightingController_->Update();

	// プレイヤーの更新処理
	if (isTitleLocked_) {
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

	// プレイヤーの攻撃力に応じてAttackDownエフェクトの透明度を設定
	if (player_ && playerAttackDownEffect_) {
		float power = static_cast<float>(player_->GetAttackDownPower());
		float minP = player_->GetAttackDownMinPower();
		float maxP = player_->GetAttackDownMaxPower();
		float alpha = 0.0f;
		if (maxP - minP > 1e-6f) {
			alpha = (power - minP) / (maxP - minP);
			alpha = std::clamp(alpha, 0.0f, 1.0f);
		}
		playerAttackDownEffect_->SetAlpha(alpha);
	}

	// ロックオン: 入力が有効ならプレイヤーとボスの位置をターゲットに設定
	bool prevLockOn = isBossLockOn_;
	if (!isTitleLocked_) {
		if (context_->inputCommand->IsCommandActive("LockOnBoss")) {
			isBossLockOn_ = !isBossLockOn_;
		}

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

    // Letterbox easing control when lock-on toggles
    if (letterbox_) {
        // Detect state change to set new animation
        if (prevLockOn != isBossLockOn_) {
            letterboxAnimTimer_ = 0.0f;
            letterboxStartHeight_ = letterboxEndHeight_;
            letterboxEndHeight_ = isBossLockOn_ ? 64.0f : 0.0f;
        }
        // Progress animation
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
	float desiredFov = 0.7f; // 通常
	if (player_->IsRushing()) {
		desiredFov = 1.0f; // 突進中
	} else if (player_->IsCharging() || player_->IsPreRushing()) {
		// 突進レベルに応じてFOVを変化させる
		int rushLevel = player_->GetRushChargeLevel();
		desiredFov = 0.4f + static_cast<float>(3 - rushLevel) * 0.1f;
	}
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

	if (isTitleLocked_) {
		cameraController_->Update(nullptr, context_->input);
	} else {
		cameraController_->Update(context_->inputCommand, context_->input);
	}
	mainCamera_->SetCamera(cameraController_->GetCamera());

	//========================================
	// 敵の更新処理
	//========================================
#pragma region EnemyUpdate
	if (!isTitleLocked_) {
		// 敵の移動処理
		bossEnemy_->Update(player_->GetPlayerPos());
		enemyAttackManager_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());
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

	// ボスがヒットした時の演出
	if (bossEnemy_->IsHit()) {
		bossEnemyModel_->SetDefaultColor({ 1.0f,1.0f,1.0f,bossEnemy_->GetAlpha() });
	}

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

	// ステージの更新処理
	stageManager_->Update();

	// 空気を演出するためのパーティクル
	airParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// ボス撃破時のフェード処理
	if (bossEnemy_->GetCurrentHP() <= 0) {
		if (!isBossDestroyFade_) {
			bossDestroyFade_->SetActive();
			isBossDestroyFade_ = true;
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
	if (bossEnemy_->isFinished()) {
		if (!clearUI_->IsActive()) {
			clearUI_->SetIsActice(true);
		}	
	}
	clearUI_->Update();

#ifdef _DEBUG
	// 地面マテリアルの更新処理
	terrain_->Update();
	bgRock_->Update();
#endif

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

	// 岩を描画
	CustomRenderer::PreDraw(CustomRenderMode::Rock);
	CustomRenderer::DrawRock(bgIceRockModel_, bgRock_->GetWorldTransform(), sceneLightingController_->GetResource(), bgRock_->GetMaterial());

	// アニメーションの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::AnimationModel);

	// プレイヤーのアニメーションを描画
	ModelRenderer::DrawAnimationWithLight(playerModel_, player_->GetWorldTransform(), sceneLightingController_->GetResource());
	// プレイヤーの影を描画する
	ModelRenderer::DrawAnimation(playerModel_, playerShadow_->GetWorldTransform(), &playerShadow_->GetMaterial());

	// 敵を描画
	ModelRenderer::DrawAnimationWithLight(bossEnemyModel_, bossEnemy_->GetWorldTransform(), sceneLightingController_->GetResource());
	// 敵の影を描画する
	ModelRenderer::DrawAnimation(bossEnemyModel_, bossEnemyShadow_->GetWorldTransform(), &bossEnemyShadow_->GetMaterial());

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

	CustomRenderer::PreDraw(CustomRenderMode::RockBoth);
	// 氷柱のモデルを描画
	const std::list<std::unique_ptr<IceFall>> &iceFalls = enemyAttackManager_->GetIceFalls();
	for (auto &iceFall : iceFalls) {
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

	// プレイヤーの攻撃演出を描画
	if (playerAttackEffect_->IsActive()) {
		ModelRenderer::Draw(planeModel_, playerAttackEffect_->GetWorldTransforms(), &playerAttackEffect_->GetMaterial());
	}

	// インスタンシング描画前処理
	ModelRenderer::PreDraw(RenderMode3D::InstancingBothNone);
	// ボスの風攻撃を描画
	ModelRenderer::DrawInstancing(windModel_, enemyWindShadowAttackParticle_->GetCurrentNumInstance(), *enemyWindShadowAttackParticle_->GetWorldTransforms());

	// インスタンシング描画前処理
	ModelRenderer::PreDraw(RenderMode3D::InstancingBoth);
	// ボスの風攻撃を描画
	ModelRenderer::DrawInstancing(windModel_, enemyWindAttackParticle_->GetCurrentNumInstance(), *enemyWindAttackParticle_->GetWorldTransforms());

	// インスタンシング描画前処理
	ModelRenderer::PreDraw(RenderMode3D::Instancing);

	// 壁破壊のパーティクルを描画
	for (auto& particle : stageManager_->GetBreakWallParticles()) {
		ModelRenderer::DrawInstancing(wallModel_, particle->GetCurrentNumInstance(), *particle->GetWorldTransforms());
	}

	// ボスの翼の演出を描画
	ModelRenderer::DrawInstancing(wingModel_, enemyWingsParticleParticle_->GetCurrentNumInstance(), *enemyWingsParticleParticle_->GetWorldTransforms());

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


#ifdef _DEBUG

	// デバック描画
	debugRenderer_->DrawAll(isDebugView ? context_->debugCamera_->GetVPMatrix() : mainCamera_->GetVPMatrix());
#endif
}

void TDGameScene::DrawUI() {

	//======================================================
	// 2D描画
	//======================================================

	// 画像の描画前処理
	SpriteRenderer::PreDraw(RenderMode2D::Normal);

	if (letterbox_) {
		if (auto s = letterbox_->GetTopSprite()) { SpriteRenderer::Draw(s, 0); }
		if (auto s = letterbox_->GetBottomSprite()) { SpriteRenderer::Draw(s, 0); }
	}

	// タイトル描画
	SpriteRenderer::Draw(playGuideSprite_.get(), playGuideGH_);

	// タイトルスプライトの描画（TitleSceneからコピー）
	if (titleSprite_) {
		SpriteRenderer::Draw(titleSprite_.get(), titleGH_);
	}
	if (spaceSprite_) {
		SpriteRenderer::Draw(spaceSprite_.get(), spaceGH_);
	}

	// ボスのHPUIを表示
	SpriteRenderer::Draw(bossHpUI_->GetFrameSprite(), 0);
	SpriteRenderer::Draw(bossHpUI_->GetEffectSprite(), 0);
	SpriteRenderer::Draw(bossHpUI_->GetSprite(), 0);
	SpriteRenderer::Draw(bossHpUI_->GetNameSprite(), bossNameGH_);

	// プレイヤーのHPUIを表示
    for (const auto &sprite : playerHpUI_->GetHpSprites()) {
		SpriteRenderer::Draw(sprite.get(), 0);
    }

	// GameOverUI描画
	if (gameOverUI_->IsActive()) {
		SpriteRenderer::Draw(gameOverUI_->GetBgSprite(), gameOverUI_->GetBgGH());
		SpriteRenderer::Draw(gameOverUI_->GetLogoSprite(), gameOverUI_->GetLogoGH());
		SpriteRenderer::Draw(gameOverUI_->GetRetrySprite(), gameOverUI_->GetRetryGH());
		SpriteRenderer::Draw(gameOverUI_->GetTitleSprite(), gameOverUI_->GetTitleGH());
	}

	// クリアUIを描画
	if (clearUI_->IsActive()) {
		SpriteRenderer::Draw(clearUI_->GetBgSprite(), 0);
		SpriteRenderer::Draw(clearUI_->GetClearSprite(), clearUI_->GetClearTexture());
		SpriteRenderer::Draw(clearUI_->GetGuideSprite(), clearUI_->GetGuidTexture());
	}

	// ボスの撃破時のフェード処理
	if (bossDestroyFade_->IsActive()) {
		SpriteRenderer::Draw(bossDestroyFade_->GetSprite(), 0);
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
	context_->inputCommand->RegisterCommand("Start", { {InputState::KeyTrigger, DIK_RETURN}, {InputState::PadTrigger, XINPUT_GAMEPAD_A},{InputState::KeyTrigger, DIK_SPACE} });
	// メニュー移動
	context_->inputCommand->RegisterCommand("Up", { {InputState::KeyTrigger, DIK_UP}, {InputState::PadTrigger, XINPUT_GAMEPAD_DPAD_UP} });
	context_->inputCommand->RegisterCommand("Down", { {InputState::KeyTrigger, DIK_DOWN}, {InputState::PadTrigger, XINPUT_GAMEPAD_DPAD_DOWN} });
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
	collisionManager_->AddCollider(bossEnemy_->GetCollider());
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