#include"ALGameScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"SpriteRenderer.h"
#include"GameParamEditor.h"
#include"FPSCounter.h"
#include"LogManager.h"
#include"Extension/CustomRenderer.h"
#include<numbers>
#include"AudioManager.h"
using namespace GameEngine;

ALGameScene::~ALGameScene() {
}

void ALGameScene::Initialize(SceneContext* context) {
	// ゲームシーンに必要な低レイヤー機能
#pragma region SceneSystem 
	// エンジン機能を取得
	context_ = context;

	// 登録するパラメータを設定
	GameParamEditor::GetInstance()->SetActiveScene("ALGameScene");

	// 初期化されたことのログを出す
	Log("Initialize : ALGameScene", "ALGameScene");

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

	// カメラをコントロールするクラスを初期化
	followCameraController_ = std::make_unique<FollowCameraController>();
	followCameraController_->Initialize();

	// ライトの生成
	sceneLightingController_ = std::make_unique<SceneLightingController>();
	sceneLightingController_->Initialize(context_->graphicsDevice->GetDevice());

	//====================================================
	// 地形の初期化
	//====================================================
#pragma region Terrain
	// 天球モデルを生成
	skyDomeModel_ = context_->modelManager->GetNameByModel("SkyDome");
	skyDomeWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// 地面を生成
	// 氷の地面を描画するためのモデル
	icePlaneModel_ = context_->modelManager->GetNameByModel("PlaneXZ");
	// 地面を生成する
	terrain_ = std::make_unique<Terrain>();
	terrain_->Initialize(context_->textureManager->GetHandleByName("grass.png"),
		context_->textureManager->GetHandleByName("ice.png"), context_->textureManager->GetHandleByName("iceNormal.png"));

	// 壁を生成
	wallModel_ = context_->modelManager->GetNameByModel("Wall");
	wallModel_->SetDefaultIsEnableLight(true);
	stageManager_ = std::make_unique<StageManager>();
	stageManager_->Initialize();

	// 空気を演出するためのパーティクル
	airParticle_ = std::make_unique<ParticleBehavior>();
	airParticle_->Initialize("AirParticle", 128);
	airParticle_->Emit({ 0.0f,0.0f,0.0f });
#pragma endregion

	//================================================================
	// プレイヤーの初期化
	//================================================================
#pragma region Player

	// 武器を初期化
	swordModel_ = context_->modelManager->GetNameByModel("Hammer");
	swordModel_->SetDefaultIsEnableLight(true);
	swordModel_->SetDefaultColor({ 1.0f,1.0f,1.0f,1.0f });
	// 武器
	playerSword_ = std::make_unique<Sword>();
	playerSword_->Initialize();

	// プレイヤーモデルを生成
	playerModel_ = context_->modelManager->GetNameByModel("Player");
	playerModel_->SetDefaultIsEnableLight(true);
	// プレイヤークラスを初期化
	player_ = std::make_unique<Player>();
	player_->SetWeapon(playerSword_.get());
	player_->Initialize(context_->inputCommand);

	// プレイヤーの壁を生成
	playerShadow_ = std::make_unique<PlaneProjectionShadow>();
	playerShadow_->Initialize(&player_->GetWorldTransform());

	// パーティクルのシステムを初期化
	playerMoveParticle_ = std::make_unique<ParticleBehavior>();
	playerMoveParticle_->Initialize("PlayerSmokeParticle", 48);
	playerMoveParticle_->Emit({ 0.0f,0.0f,0.0f });
	// 平面モデル
	planeModel_ = context_->modelManager->GetNameByModel("Plane");

	// ヒットダメージ演出
	hitEffectParticle_ = std::make_unique<ParticleBehavior>();
	hitEffectParticle_->Initialize("HitEffect", 32);

	// 攻撃演出
	attackEffectParticle_ = std::make_unique<ParticleBehavior>();
	attackEffectParticle_->Initialize("PlayerAttackEffect", 32);
	// 攻撃演出のアクセント
	attackAccentEffectParticle_ = std::make_unique<ParticleBehavior>();
	attackAccentEffectParticle_->Initialize("PlayerAttackAccentEffect", 32);
#pragma endregion

	//========================================================
	// 敵の初期化
	//========================================================
#pragma region Enemy

	// ボスモデルを生成
	bossEnemyModel_ = context_->modelManager->GetNameByModel("BossBody");
	bossEnemyEyeModel_ = context_->modelManager->GetNameByModel("BossEye");
	bossEnemyEyeModel_->SetDefaultColor({ 0.0f,0.0f,0.0f,1.0f });

	// 敵の弾モデルを生成
	rockBulletModel_ = context_->modelManager->GetNameByModel("Cube");

	// 敵の遠距離攻撃管理クラスを初期化
	enemyProjectileManager_ = std::make_unique<EnemyProjectileManager>();
	enemyProjectileManager_->Initialize();

	// ボスの移動パーティクル
	bossEnmeyMoveParticle_ = std::make_unique<ParticleBehavior>();
	bossEnmeyMoveParticle_->Initialize("BossSmokeParticle", 32);
	bossEnmeyMoveParticle_->Emit({ 0.0f,0.0f,0.0f });

	// ボス敵クラスを初期化
	bossEnemy_ = std::make_unique<BossEnemy>();
	bossEnemy_->Initialize(enemyProjectileManager_.get(), context_->textureManager->GetHandleByName("boss.png"));

	// ボス敵の影を生成
	bossEnemyShadow_ = std::make_unique<PlaneProjectionShadow>();
	bossEnemyShadow_->Initialize(&bossEnemy_->GetWorldTransform());
#pragma endregion

	//==================================================
	// UIの初期化
	//==================================================
#pragma region UI
	// ボスのhpUIを初期化
	bossHpUI_ = std::make_unique<BossHpUI>();
	bossHpUI_->Initialize(bossEnemy_->GetMaxHp());

	// プレイヤーのhpUIを初期化
	playerHpUI_ = std::make_unique<PlayerHpUI>();
	playerHpUI_->Initialize(player_->GetMaxHp(), context_->textureManager);

	// 操作説明UI
	guideSprite_ = Sprite::Create({ 928.0f,534.0f }, { 320.0f,170.0f }, { 0.0f,0.0f }, { 0.1f,0.1f,0.1f,1.0f });
	guideGH_ = context_->textureManager->GetHandleByName("guide.png");

	// ゲームオーバーUIを初期化
	gameOverUI_ = std::make_unique<GameOverUI>();
	gameOverUI_->Initialize(context_->inputCommand, context_->textureManager);

	// リトライ
	gameOverUI_->SetRetry([this]() {
		isFinished_ = true;
		sceneState_ = SceneState::ALGame;

		// BGMを停止
		if (AudioManager::GetInstance().IsPlay(gameSH_)) {
			AudioManager::GetInstance().Stop(gameSH_);
		}
	});
	// タイトル
	gameOverUI_->SetTitle([this]() {
		isFinished_ = true;
		sceneState_ = SceneState::Title;

		// BGMを停止
		if (AudioManager::GetInstance().IsPlay(gameSH_)) {
			AudioManager::GetInstance().Stop(gameSH_);
		}
	});
#pragma endregion

	// クリアまでの時間を計測する
	clearTimeTracker_ = std::make_unique<ClearTimeTracker>();
	// 時間の計測を開始
	clearTimeTracker_->StartMeasureTimes();

	// bgmを取得
	gameSH_ = AudioManager::GetInstance().GetHandleByName("gameBGM.mp3");
}

void ALGameScene::Update() {

	// BGMを再生
	if (!AudioManager::GetInstance().IsPlay(gameSH_)) {
		AudioManager::GetInstance().Play(gameSH_, 0.3f, false);
	}

	if (isGameOver_) {
		// ゲームオーバーの更新処理
		gameOverUI_->Update();
	} else {
		// ゲームプレイの更新処理
		GamePlayUpdate();
	}
}

void ALGameScene::Draw(const bool& isDebugView) {

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

	// 氷のテスト描画
	CustomRenderer::PreDraw(CustomRenderMode::Ice);
	CustomRenderer::DrawIce(icePlaneModel_, terrain_->GetWorldTransform(), sceneLightingController_->GetResource(), terrain_->GetMaterial());

	CustomRenderer::PreDraw(CustomRenderMode::RockBoth);

	// 壁を描画
	for (auto& wall : stageManager_->GetWalls()) {
		CustomRenderer::DrawRock(wallModel_, wall->GetWorldTransform(), sceneLightingController_->GetResource(), wall->GetMaterial());
	}

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// プレイヤーを描画
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(playerModel_, player_->GetWorldTransform());
	// プレイヤーの影を描画
	ModelRenderer::Draw(playerModel_, playerShadow_->GetWorldTransform(), &playerShadow_->GetMaterial());
	// 剣を描画
	if (player_->GetPlayerBehavior() == Player::Behavior::Attack) {
		ModelRenderer::DrawLight(sceneLightingController_->GetResource());
		ModelRenderer::Draw(swordModel_, playerSword_->GetWorldTransform());
	}

	// ボスの体を描画
	CustomRenderer::PreDraw(CustomRenderMode::RockBoth);
	CustomRenderer::DrawRock(bossEnemyModel_, bossEnemy_->GetWorldTransform(), sceneLightingController_->GetResource(), bossEnemy_->GetMaterial());
	// ボスの目を描画
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);
	ModelRenderer::Draw(bossEnemyEyeModel_, bossEnemy_->GetWorldTransform());
	// ボス敵の影を描画
	ModelRenderer::Draw(bossEnemyModel_, bossEnemyShadow_->GetWorldTransform(), &bossEnemyShadow_->GetMaterial());

	// 敵の岩の弾を描画
	for (auto rockBullet : enemyProjectileManager_->GetProjectilesByType(ProjectileType::Rock)) {
		ModelRenderer::Draw(rockBulletModel_, rockBullet->GetWorldTransform());
	}

	// 複数モデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::InstancingScreen);

	// 攻撃の演出を描画
	ModelRenderer::DrawInstancing(planeModel_, attackEffectParticle_->GetCurrentNumInstance(), *attackEffectParticle_->GetWorldTransforms());

	// 複数モデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::InstancingAdd);

	// プレイヤーの移動パーティクルを描画
	ModelRenderer::DrawInstancing(planeModel_, playerMoveParticle_->GetCurrentNumInstance(), *playerMoveParticle_->GetWorldTransforms());

	// ボスの移動パーティクルを描画
	ModelRenderer::DrawInstancing(planeModel_, bossEnmeyMoveParticle_->GetCurrentNumInstance(), *bossEnmeyMoveParticle_->GetWorldTransforms());

	// ヒットエフェクトの演出の描画
	ModelRenderer::DrawInstancing(planeModel_, hitEffectParticle_->GetCurrentNumInstance(), *hitEffectParticle_->GetWorldTransforms());

	// 空気を演出するためのパーティクルを描画
	ModelRenderer::DrawInstancing(planeModel_, airParticle_->GetCurrentNumInstance(), *airParticle_->GetWorldTransforms());

	// 攻撃のアクセント演出を描画
	ModelRenderer::DrawInstancing(planeModel_, attackAccentEffectParticle_->GetCurrentNumInstance(), *attackAccentEffectParticle_->GetWorldTransforms());

#ifdef USE_IMGUI

	// デバック描画
	debugRenderer_->DrawAll(isDebugView ? context_->debugCamera_->GetVPMatrix() : mainCamera_->GetVPMatrix());
#endif

	//========================================================================
	// 2D描画
	//========================================================================

	// 画像の描画前処理
	SpriteRenderer::PreDraw(RenderMode2D::Normal);

	// ボスのHPUIを表示
	SpriteRenderer::Draw(bossHpUI_->GetFrameSprite(), 0);
	SpriteRenderer::Draw(bossHpUI_->GetEffectSprite(), 0);
	SpriteRenderer::Draw(bossHpUI_->GetSprite(), 0);

	// プレイヤーのHpUIを表示
	for (const auto& sprite : playerHpUI_->GetHpSprites()) {
		SpriteRenderer::Draw(sprite.get(), playerHpUI_->GetHpGH());
	}

	// 操作説明を表示
	SpriteRenderer::Draw(guideSprite_.get(), guideGH_);

	// ゲームオーバーシーンの描画
	if (isGameOver_) {
		SpriteRenderer::Draw(gameOverUI_->GetBgSprite(), gameOverUI_->GetBgGH());
		SpriteRenderer::Draw(gameOverUI_->GetLogoSprite(), gameOverUI_->GetLogoGH());
		SpriteRenderer::Draw(gameOverUI_->GetRetrySprite(), gameOverUI_->GetRetryGH());
		SpriteRenderer::Draw(gameOverUI_->GetTitleSprite(), gameOverUI_->GetTitleGH());
	}
}

void ALGameScene::InputRegisterCommand() {
	// 移動の入力コマンドを登録する
	context_->inputCommand->RegisterCommand("MoveUp", { {InputState::KeyPush, DIK_W },{InputState::PadLeftStick,0,{0.0f,1.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_UP } });
	context_->inputCommand->RegisterCommand("MoveDown", { {InputState::KeyPush, DIK_S },{InputState::PadLeftStick,0,{0.0f,-1.0f},0.2f}, {InputState::PadPush, XINPUT_GAMEPAD_DPAD_DOWN} });
	context_->inputCommand->RegisterCommand("MoveLeft", { {InputState::KeyPush, DIK_A },{InputState::PadLeftStick,0,{-1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_LEFT } });
	context_->inputCommand->RegisterCommand("MoveRight", { {InputState::KeyPush, DIK_D },{InputState::PadLeftStick,0,{1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_RIGHT } });
	// ジャンプコマンドを登録する
	context_->inputCommand->RegisterCommand("Jump", { {InputState::KeyTrigger, DIK_SPACE},{InputState::PadTrigger, XINPUT_GAMEPAD_A} });
	context_->inputCommand->RegisterCommand("Attack", { {InputState::KeyTrigger, DIK_K},{InputState::PadTrigger, XINPUT_GAMEPAD_X} });
	context_->inputCommand->RegisterCommand("Dush", { {InputState::KeyTrigger, DIK_J},{InputState::PadTrigger, XINPUT_GAMEPAD_B} });

	// カメラ操作のコマンドを登録する
	context_->inputCommand->RegisterCommand("CameraMoveLeft", { { InputState::KeyPush, DIK_LEFT },{InputState::PadRightStick,0,{-1.0f,0.0f},0.2f} });
	context_->inputCommand->RegisterCommand("CameraMoveRight", { { InputState::KeyPush, DIK_RIGHT },{InputState::PadRightStick,0,{1.0f,0.0f},0.2f} });

	context_->inputCommand->RegisterCommand("CameraLockOn", { {InputState::KeyTrigger, DIK_G},{InputState::PadTrigger, XINPUT_GAMEPAD_Y} });

	// 決定ボタン
	context_->inputCommand->RegisterCommand("Decision", { {InputState::KeyTrigger, DIK_SPACE},{InputState::PadTrigger, XINPUT_GAMEPAD_A} });
}

void ALGameScene::GamePlayUpdate() {
	// ボスの体力がなくなったらシーンを切り替える
	if (!bossEnemy_->GetIsAlive()) {
		isFinished_ = true;

		// 時間の計測を終了
		clearTimeTracker_->EndMeasureTimes();

		// BGMを停止
		if (AudioManager::GetInstance().IsPlay(gameSH_)) {
			AudioManager::GetInstance().Stop(gameSH_);
		}
	}

	// プレイヤーの対りょっくがなくなったらゲームオーバーに切り替え
	if (player_->GetHp() <= 0) {
		isGameOver_ = true;
	}

	// 当たり判定のリストを削除
	collisionManager_->ClearList();
	// デバックリストを削除
	debugRenderer_->Clear();

	// ライトの更新処理
	sceneLightingController_->Update();

	//============================================
	// プレイヤーの更新処理
	//============================================
#pragma region PlayerUpdate
	// プレイヤーの更新処理
	player_->SetCameraInfo(followCameraController_->GetRotateMatrix(), followCameraController_->GetIsLockOn(), bossEnemy_->GetPosition());
	player_->Update();

	// プレイヤーの影の更新処理
	playerShadow_->Update();

	// パーティクルの更新処理
	playerMoveParticle_->SetEmitterPos(player_->GetPlayerPos());
	playerMoveParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());
#pragma endregion

	//===========================================
	// 敵の更新処理
	//===========================================
#pragma region EnemyUpdate
	// ボス敵の更新処理
	bossEnemy_->Update(player_->GetPlayerPos());
	//bossEnemyModel_->SetDefaultColor({ 1.0f,1.0f,1.0f,bossEnemy_->GetAlpha() });

	// ボス敵の影の更新処理
	bossEnemyShadow_->Update();

	// ボスの移動パーティクル
	bossEnmeyMoveParticle_->SetEmitterPos(bossEnemy_->GetPosition());
	bossEnmeyMoveParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// 敵の遠距離攻撃の更新処理
	enemyProjectileManager_->Update();
#pragma endregion

	//======================================================
	// 当たり判定の更新処理
	//======================================================
	UpdateCollision();
	
	//===================================================
	// ヒット演出の更新処理
	//===================================================
#pragma region HitEffectUpdate
	// ヒットした時のエフェクト
	if (player_->IsHit()) {
		hitEffectParticle_->Emit(player_->GetPlayerPos());
	}
	hitEffectParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// 攻撃した時のエフェクト
	if (player_->IsAttack()) {
		attackEffectParticle_->Emit(player_->GetPlayerPos());
		attackAccentEffectParticle_->Emit(player_->GetPlayerPos());
	}
	// 攻撃演出の更新処理
	attackEffectParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());
	attackAccentEffectParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());
#pragma endregion

	//=====================================================================
	// 地形関係の更新処理
	//=====================================================================
#pragma region TerrainUpdate
	airTimer_ += FpsCounter::deltaTime / maxAirTime_;
	if (airTimer_ >= 1.0f) {
		airParticle_->SetFieldAcceleration(Vector3(airSpeed_, -1.0f, 0.0f));
		airSpeed_ *= -1.0f;
		airTimer_ = 0.0f;
	}
	// 空気を演出するためのパーティクル
	airParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());
#pragma endregion

	//==================================================================
	// カメラの更新処理
	//==================================================================
#pragma region CameraUpdate
	// カメラコントロールの更新処理
	followCameraController_->Update(context_->inputCommand);
	// プレイヤーの要素をカメラに送る
	followCameraController_->SetFollowPos(player_->GetPlayerPos(), player_->GetVelocity());
	// カメラのターゲット目標を設定
	followCameraController_->SetLockOnPos(bossEnemy_->GetPosition());

	// カメラの更新処理
	mainCamera_->SetCamera(followCameraController_->GetCamera());
#pragma endregion

	//============================================================
	// UIの更新処理
	//============================================================
#pragma region UIUpdate
	// ボスのHpUIの更新処理
	bossHpUI_->SetCurrentHp(bossEnemy_->GetCurrentHp());
	bossHpUI_->Update();

	// プレイヤーのHpUIの更新処理
	playerHpUI_->SetCurrentHp(player_->GetHp());
	playerHpUI_->Update();
#pragma endregion
}

void ALGameScene::UpdateCollision() {

	// プレイヤーの当たり判定を登録する
	collisionManager_->AddCollider(player_->GetCollider());
#ifdef USE_IMGUI
	// デバックデータを取得する
	debugRenderer_->AddSphere(player_->GetSphereData());
#endif
	if (player_->GetPlayerBehavior() == Player::Behavior::Attack) {
		// 武器
		auto weapon = player_->GetWeapon();
		for (auto& collider : weapon->GetColliders()) {
			collisionManager_->AddCollider(collider.get());
#ifdef USE_IMGUI
			debugRenderer_->AddSphere({ collider->GetWorldPosition(),collider->GetRadius() });
#endif
		}
	}

	// ボス敵の当たり判定を登録する
	collisionManager_->AddCollider(bossEnemy_->GetCollider());
#ifdef USE_IMGUI
	// デバックデータを取得する
	debugRenderer_->AddSphere(bossEnemy_->GetSphereData());
#endif

	// 壁の当たり判定を登録する
	for (auto& wall : stageManager_->GetWalls()) {
		collisionManager_->AddCollider(wall->GetCollider());
#ifdef USE_IMGUI
		// デバックデータを取得する
		debugRenderer_->AddBox(wall->GetAABBData());
#endif
	}

	// 衝突判定
	collisionManager_->CheckAllCollisions();
}

void ALGameScene::DebugUpdate() {

	for (auto& wall : stageManager_->GetWalls()) {
		wall->Update();
	}
}
