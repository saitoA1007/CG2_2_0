#include"TDGameScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"SpriteRenderer.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
#include"LogManager.h"
#include<numbers>

#include"Extension/CustomRenderer.h"

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

	// 天球モデルを生成
	skyDomeModel_ = context_->modelManager->GetNameByModel("SkyDome");
	skyDomeWorldTransform_.Initialize({{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}});

	// 地面を生成
	// 氷の地面を描画するためのモデル
	icePlaneModel_ = context_->modelManager->GetNameByModel("PlaneXZ");
	// 地面を生成する
	terrain_ = std::make_unique<Terrain>();
	terrain_->Initialize(context_->textureManager->GetHandleByName("gras.png"),
		context_->textureManager->GetHandleByName("ice.png"), context_->textureManager->GetHandleByName("iceNormal.png"));

	// ライトの生成
	sceneLightingController_ = std::make_unique<SceneLightingController>();
	sceneLightingController_->Initialize(context_->graphicsDevice->GetDevice());

	// 壁を生成する
	wallModel_ = context_->modelManager->GetNameByModel("Wall");
	// ステージを生成を初期化
	stageManager_ = std::make_unique<StageManager>();
	stageManager_->Initialize();
	stageManager_->Update();

	// StageWallPlane用モデル
	stageWallPlaneModel_ = context_->modelManager->GetNameByModel("PlaneXZ");

	// Create a single shared material for all StageWallPlane and copy terrain textures
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
		// Align StageWallPlane transforms with StageManager's Wall objects
		auto& walls = stageManager_->GetWalls();
		size_t count = std::min(stageWallPlanes_.size(), walls.size());
		for (size_t i = 0; i < count; ++i) {
			// Use the same transform as the Wall but flip Y rotation by 180 degrees
			const Transform& wallTransform = walls[i]->GetWorldTransform().transform_;
			Transform t = wallTransform;
			// add PI to Y rotation to reverse facing
			t.rotate.y += static_cast<float>(std::numbers::pi);
			// XZ平明のモデルを使うので、壁として使うために回転
            t.rotate.x -= static_cast<float>(std::numbers::pi) / 2.0f;
			t.scale.z = 32.0f;
			t.translate.y = 16.0f;
			stageWallPlanes_[i].Initialilze(t);
		}
		// If there are more StageWallPlane entries than walls, initialize remaining with default invisible scale
		for (size_t i = count; i < stageWallPlanes_.size(); ++i) {
			Transform t; // default unit transform
			t.scale = { 0.0f,0.0f,0.0f };
			stageWallPlanes_[i].Initialilze(t);
		}
	}

	// プレイヤーモデルを生成
	playerModel_ = context_->modelManager->GetNameByModel("Triangular");
	playerModel_->SetDefaultIsEnableLight(true);
	// プレイヤークラスを初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();
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

	// 敵の処理に関する初期化処理
#pragma region EnemySystem 
	// 敵の攻撃管理クラス
	enemyAttackManager_ = std::make_unique<EnemyAttackManager>();
	enemyAttackManager_->Initialize(context_->postEffectManager_);
	// ボス敵モデルを生成
	bossEnemyModel_ = context_->modelManager->GetNameByModel("Boss");
	//bossEnemyModel_->SetDefaultColor({ 1.0f,0.0f,0.0f,1.0f });
	bossEnemyModel_->SetDefaultIsEnableLight(true);
	// ボス敵クラスを初期化
	bossEnemy_ = std::make_unique<BossEnemy>();
	bossEnemy_->Initialize(stageManager_->GetRadius(), enemyAttackManager_.get(), debugRenderer_.get());

	// ボスのアニメーションデータを取得する
	bossEnemyAnimationData_ = context_->animationManager->GetNameByAnimations("BossBirdBaseMove");
	// ボスのアニメーションの再生を管理する
	bossEnemyAnimator_ = std::make_unique<Animator>();
	bossEnemyAnimator_->Initialize(bossEnemyModel_, &bossEnemyAnimationData_["基本移動"]);
	// ボスの影
	bossEnemyShadow_ = std::make_unique<PlaneProjectionShadow>();
	bossEnemyShadow_->Initialize(&bossEnemy_->GetWorldTransform());

	// ボスの突進攻撃の初期化
	enemyRushEffect_ = std::make_unique<EnemyRushEffect>();
	enemyRushEffect_->Initialize();
	enemyRushEffect_->SetParent(&bossEnemy_->GetWorldTransform());

	// ボスの風攻撃の初期化
	enemyWindAttackParticle_ = std::make_unique<EnemyWindAttackParticle>();
	enemyWindAttackParticle_->Initialize(context_->textureManager->GetHandleByName("noise.png"));

	// 氷柱のモデルを取得
	iceFallModel_ = context_->modelManager->GetNameByModel("IceFall");
	iceFallModel_->SetDefaultIsEnableLight(true);
	// 突進攻撃演出モデル
	enemyRushModel_ = context_->modelManager->GetNameByModel("RushWave");
	// 風攻撃演出モデル
	windModel_ = context_->modelManager->GetNameByModel("Wind");
#pragma endregion

	// 平面モデルを取得
	planeModel_ = context_->modelManager->GetNameByModel("Plane");

	//==================================================
	// カメラアニメーション設定
	//==================================================

	{
		// 指定のキーとイージング関数を設定する
		std::vector<AnimationKeyframe<Vector3>> positionKeys;
		std::vector<AnimationKeyframe<Vector3>> rotateKeys;
		std::vector<AnimationKeyframe<Vector3>> lookAtKeys;
		std::vector<AnimationKeyframe<float>> fovKeys;

		// イージングラッパー
		auto vecEase = [](const Vector3 &a, const Vector3 &b, float t) -> Vector3 { return EaseInOutCubic(a, b, t); };
		auto floatEase = [](const float &a, const float &b, float t) -> float { return EaseInOutCubic(a, b, t); };

		float pitch;
        float yaw;
		const float degToRad = static_cast<float>(std::numbers::pi) / 180.0f;

		// t=0.5f のキー
        pitch = -15.0f * degToRad;
		positionKeys.push_back(AnimationKeyframe<Vector3>{ 0.5f, Vector3{0.0f, 0.0f, 8.0f}, vecEase });
		rotateKeys.push_back(AnimationKeyframe<Vector3>{ 0.5f, Vector3{ pitch, 0.0f, 0.0f }, vecEase });
		lookAtKeys.push_back(AnimationKeyframe<Vector3>{ 0.5f, Vector3{ 0.0f, 0.0f, 0.0f }, vecEase });
		fovKeys.push_back(AnimationKeyframe<float>{ 0.5f, 0.7f, floatEase });

		// t=2.0f のキー
        pitch = -45.0f * degToRad;
		yaw = 360.0f * degToRad;
		positionKeys.push_back(AnimationKeyframe<Vector3>{ 2.0f, Vector3{0.0f, 0.0f, 32.0f}, vecEase });
		rotateKeys.push_back(AnimationKeyframe<Vector3>{ 2.0f, Vector3{ pitch, yaw, 0.0f }, vecEase });
		lookAtKeys.push_back(AnimationKeyframe<Vector3>{ 2.0f, Vector3{ 0.0f, 0.0f, 0.0f }, vecEase });
		fovKeys.push_back(AnimationKeyframe<float>{ 2.0f, 1.5f, floatEase });

		cameraController_->SetAnimationKeyframes(positionKeys, rotateKeys, lookAtKeys, fovKeys);
		cameraController_->PlayAnimation();
		cameraController_->StartCameraShake(0.5f, 10.0f, 20.0f,
            [](const Vector3 &a, const Vector3 &b, float t) { return EaseInOutSine(a, b, t); },
			CameraController::ShakeOrigin::TargetPosition,
			false, true, true, true);
	}

	// 入力コマンドを設定する
	InputRegisterCommand();
}

void TDGameScene::Update() {

	if (context_->input->TriggerKey(DIK_U)) {
		isFinished_ = true;
	}

	// デバックリストを削除
	debugRenderer_->Clear();

	// 当たり判定をリセット
	collisionManager_->ClearList();

	// ライトの更新処理
	sceneLightingController_->Update();

	// プレイヤーの更新処理
	player_->Update(context_->inputCommand, cameraController_->GetCamera());
	// プレイヤーの影の更新処理
	playerShadow_->Update();

	// ロックオン: 入力が有効ならプレイヤーとボスの位置をターゲットに設定
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

	//============================
	// FOV設定
	//============================
	float desiredFov = 0.7f; // 通常
	if (player_->IsRushing()) {
		desiredFov = 1.0f; // 突進中
	} else if (player_->IsCharging() || player_->IsPreRushing()) {
		// 溜め比率 0.0 -> 0.6f, 1.0 -> 0.4f に線形補間
		float chargeRatio = player_->GetChargeRatio();
        desiredFov = Lerp(0.6f, 0.4f, chargeRatio);
	}
	cameraController_->SetDesiredFov(desiredFov);

	cameraController_->Update(context_->inputCommand, context_->input);
	mainCamera_->SetCamera(cameraController_->GetCamera());

	// 敵の移動処理
	bossEnemy_->Update(player_->GetPlayerPos());
	enemyAttackManager_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());
	bossEnemyAnimator_->Update();
	// 敵の影の更新処理
	bossEnemyShadow_->Update();
	// 突進攻撃演出の更新処理
	enemyRushEffect_->Update();

	if (enemyAttackManager_->IsWind()) {
		enemyWindAttackParticle_->SetIsLoop(true);
		enemyWindAttackParticle_->SetEmitterPos(bossEnemy_->GetWorldPosition());
		enemyWindAttackParticle_->SetVelocity(enemyAttackManager_->GetWindVelocity());
	} else {
		enemyWindAttackParticle_->SetIsLoop(false);
	}
	
	// 風攻撃演出の更新処理
	enemyWindAttackParticle_->Update();

	// ステージの更新処理
	//stageManager_->Update();

	// StageWallPlaneの更新
#ifdef _DEBUG
    StageWallPlane::ApplyDebugParam(stageWallPlaneMaterial_.get());
#endif
	for (auto &plane : stageWallPlanes_) {
		plane.Update();
	}

	// 当たり判定の更新処理
	UpdateCollision();

#ifdef _DEBUG
	// 地面マテリアルの更新処理
	terrain_->Update();
#endif
}

void TDGameScene::Draw(const bool& isDebugView) {

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

	// ステージを描画する
	stageManager_->Draw(wallModel_);

	// StageWallPlaneの描画 (IceMaterial via CustomRenderer)
	// Set CustomRenderer camera and draw
	if (isDebugView) {
		CustomRenderer::SetCamera(context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
	} else {
		CustomRenderer::SetCamera(mainCamera_->GetVPMatrix(), mainCamera_->GetCameraResource());
	}

	// プレイヤーを描画
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(playerModel_, player_->GetWorldTransform());
	// プレイヤーの影描画
	ModelRenderer::Draw(playerModel_, playerShadow_->GetWorldTransform(), &playerShadow_->GetMaterial());

	// 氷柱のモデルを描画
	const std::list<std::unique_ptr<IceFall>>& iceFalls = enemyAttackManager_->GetIceFalls();
	for (auto& iceFall : iceFalls) {
		if (iceFall->IsAlive()) {
			ModelRenderer::DrawLight(sceneLightingController_->GetResource());
			ModelRenderer::Draw(iceFallModel_, iceFall->GetWorldTransform());
			ModelRenderer::Draw(iceFallModel_, iceFall->GetShadowWorldTransform(),&iceFall->GetShadowMaterial());
		}
	}

	// アニメーションの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::AnimationModel);

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

	// 3Dモデルの両面描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModelBoth);

	// ボスの突進攻撃演出の描画
	for (auto& rushEffect : enemyRushEffect_->GetWorldTransforms()) {
		ModelRenderer::Draw(enemyRushModel_, rushEffect);
	}

	// インスタンシング描画前処理
	ModelRenderer::PreDraw(RenderMode3D::InstancingBoth);

	// ボスの風攻撃を描画
	ModelRenderer::DrawInstancing(windModel_, enemyWindAttackParticle_->GetCurrentNumInstance(), *enemyWindAttackParticle_->GetWorldTransforms());
	
	// 複数モデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::InstancingAdd);

	// 氷柱を落とすまでの演出を描画
	for (auto& iceFallEffect : enemyAttackManager_->GetIceFallEffectDatas()) {
		if (!iceFallEffect.isActive) { continue; }
		ModelRenderer::DrawInstancing(planeModel_, iceFallEffect.particle->GetCurrentNumInstance(), *iceFallEffect.particle->GetWorldTransforms());
	}

#ifdef _DEBUG

	// デバック描画
	debugRenderer_->DrawAll(isDebugView ? context_->debugCamera_->GetVPMatrix() : mainCamera_->GetVPMatrix());
#endif

	//========================================================================
	// 2D描画
	//========================================================================

	// 画像の描画前処理
	//SpriteRenderer::PreDraw(RenderMode2D::Normal);

	//SpriteRenderer::Draw(sprite_.get(), uvCheckerGH_);
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

	// ロックオンコマンド
	context_->inputCommand->RegisterCommand("LockOnBoss", { {InputState::KeyTrigger, DIK_TAB }, {InputState::PadTrigger, XINPUT_GAMEPAD_RIGHT_THUMB} });
	
	// カメラ操作のコマンドを登録する
	context_->inputCommand->RegisterCommand("CameraMoveLeft", { { InputState::KeyPush, DIK_LEFT },{InputState::PadRightStick,0,{-1.0f,0.0f},0.2f} });
	context_->inputCommand->RegisterCommand("CameraMoveRight", { { InputState::KeyPush, DIK_RIGHT },{InputState::PadRightStick,0,{1.0f,0.0f},0.2f} });
}

void TDGameScene::UpdateCollision() {
    collisionManager_->AddCollider(player_->GetCollider());
#ifdef _DEBUG
    debugRenderer_->AddSphere(player_->GetSphereData());
#endif

	// 敵の当たり判定を登録する
	collisionManager_->AddCollider(bossEnemy_->GetCollider());
#ifdef _DEBUG
	debugRenderer_->AddSphere(bossEnemy_->GetSphereData(),{1.0f,1.0f,0.0f,1.0f});
#endif

	// 生存している壁の要素を取得する
	const std::vector<Wall*> aliveWalls =  stageManager_->GetAliveWalls();
	for (auto& wall : aliveWalls) {
		// 当たり判定を追加
		collisionManager_->AddCollider(wall->GetCollider());
#ifdef _DEBUG
		// デバック描画に追加
		debugRenderer_->AddBox(wall->GetOBBData());
#endif
	}

	// 氷柱の当たり判定を登録する
	const std::list<std::unique_ptr<IceFall>>& iceFalls = enemyAttackManager_->GetIceFalls();
	for (auto& iceFall : iceFalls) {
		if (iceFall->IsAlive()) {
			collisionManager_->AddCollider(iceFall->GetCollider());
		}
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

	// 当たり判定の表示管理
	ImGui::Begin("DebugCollision");
	ImGui::Checkbox("IsDrawCollision", &isDrawCollision_);
	debugRenderer_->SetEnabled(isDrawCollision_);
	ImGui::End();
#endif
}
