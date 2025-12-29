#include"ALGameScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"SpriteRenderer.h"
#include"GameParamEditor.h"
#include"FPSCounter.h"
#include"LogManager.h"
#include<numbers>
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

	// 天球モデルを生成
	skyDomeModel_ = context_->modelManager->GetNameByModel("SkyDome");
	skyDomeWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// 地面を生成
	terrainModel_ = context_->modelManager->GetNameByModel("PlaneXZ");
	grassGH_ = context_->textureManager->GetHandleByName("grass.png");
	terrainWorldTransform_.Initialize({ {30.0f,30.0f,30.0f},{0.0f,0.0f,0.0f},{0.0f,-0.2f,0.0f} });

	// カメラをコントロールするクラスを初期化
	followCameraController_ = std::make_unique<FollowCameraController>();
	followCameraController_->Initialize();

	// ライトの生成
	sceneLightingController_ = std::make_unique<SceneLightingController>();
	sceneLightingController_->Initialize(context_->graphicsDevice->GetDevice());

	// プレイヤーモデルを生成
	playerModel_ = context_->modelManager->GetNameByModel("Triangular");
	playerModel_->SetDefaultIsEnableLight(true);
	// プレイヤークラスを初期化
	player_ = std::make_unique<Player>();
	player_->Initialize(context_->inputCommand);

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

	// ボスモデルを生成
	bossEnemyModel_ = context_->modelManager->GetNameByModel("Cube");
	bossEnemyModel_->SetDefaultIsEnableLight(true);
	bossEnemyModel_->SetDefaultColor({ 1.0f,0.0f,0.0f,1.0f });
	// ボス敵クラスを初期化
	bossEnemy_ = std::make_unique<BossEnemy>();
	bossEnemy_->Initialize();

	// ボスの移動パーティクル
	bossEnmeyMoveParticle_ = std::make_unique<ParticleBehavior>();
	bossEnmeyMoveParticle_->Initialize("BossSmokeParticle", 32);
	bossEnmeyMoveParticle_->Emit({ 0.0f,0.0f,0.0f });

	// 空気を演出するためのパーティクル
	airParticle_ = std::make_unique<ParticleBehavior>();
	airParticle_->Initialize("AirParticle", 128);
	airParticle_->Emit({ 0.0f,0.0f,0.0f });

	// ボスのhpUIを初期化
	bossHpUI_ = std::make_unique<BossHpUI>();
	bossHpUI_->Initialize(bossEnemy_->GetMaxHp());
}

void ALGameScene::Update() {

	// ゲームプレイの更新処理
	GamePlayUpdate();
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

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// 天球の描画
	ModelRenderer::Draw(skyDomeModel_, skyDomeWorldTransform_);

	// 地面を描画
	terrainModel_->SetDefaultTextureHandle(grassGH_);
	ModelRenderer::Draw(terrainModel_, terrainWorldTransform_);

	// プレイヤーを描画
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(playerModel_, player_->GetWorldTransform());

	// ボス敵を描画
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(bossEnemyModel_, bossEnemy_->GetWorldTransform());

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

#ifdef _DEBUG

	// デバック描画
	debugRenderer_->DrawAll(isDebugView ? context_->debugCamera_->GetVPMatrix() : mainCamera_->GetVPMatrix());
#endif

	//========================================================================
	// 2D描画
	//========================================================================

	// 画像の描画前処理
	SpriteRenderer::PreDraw(RenderMode2D::Normal);

	// ボスのHPUIを表示
	SpriteRenderer::Draw(bossHpUI_->GetEffectSprite(), 0);
	SpriteRenderer::Draw(bossHpUI_->GetSprite(), 0);
}

void ALGameScene::InputRegisterCommand() {
	// 移動の入力コマンドを登録する
	context_->inputCommand->RegisterCommand("MoveUp", { {InputState::KeyPush, DIK_W },{InputState::PadLeftStick,0,{0.0f,1.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_UP } });
	context_->inputCommand->RegisterCommand("MoveDown", { {InputState::KeyPush, DIK_S },{InputState::PadLeftStick,0,{0.0f,-1.0f},0.2f}, {InputState::PadPush, XINPUT_GAMEPAD_DPAD_DOWN} });
	context_->inputCommand->RegisterCommand("MoveLeft", { {InputState::KeyPush, DIK_A },{InputState::PadLeftStick,0,{-1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_LEFT } });
	context_->inputCommand->RegisterCommand("MoveRight", { {InputState::KeyPush, DIK_D },{InputState::PadLeftStick,0,{1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_RIGHT } });
	// ジャンプコマンドを登録する
	context_->inputCommand->RegisterCommand("Jump", { {InputState::KeyTrigger, DIK_SPACE},{InputState::PadTrigger, XINPUT_GAMEPAD_A} });
	context_->inputCommand->RegisterCommand("Attack", { {InputState::KeyTrigger, DIK_K},{InputState::PadTrigger, XINPUT_GAMEPAD_B} });
	context_->inputCommand->RegisterCommand("Dush", { {InputState::KeyTrigger, DIK_J},{InputState::PadTrigger, XINPUT_GAMEPAD_X} });

	// カメラ操作のコマンドを登録する
	context_->inputCommand->RegisterCommand("CameraMoveLeft", { { InputState::KeyPush, DIK_LEFT },{InputState::PadRightStick,0,{-1.0f,0.0f},0.2f} });
	context_->inputCommand->RegisterCommand("CameraMoveRight", { { InputState::KeyPush, DIK_RIGHT },{InputState::PadRightStick,0,{1.0f,0.0f},0.2f} });
}

void ALGameScene::GamePlayUpdate() {
	// ボスの体力がなくなったらシーンを切り替える
	if (!bossEnemy_->GetIsAlive()) {
		isFinished_ = true;
	}

	// 当たり判定のリストを削除
	collisionManager_->ClearList();
	// デバックリストを削除
	debugRenderer_->Clear();

	// ライトの更新処理
	sceneLightingController_->Update();

	// プレイヤーの更新処理
	player_->SetRotateMatrix(followCameraController_->GetRotateMatrix());
	player_->Update();
	// パーティクルの更新処理
	playerMoveParticle_->SetEmitterPos(player_->GetPlayerPos());
	playerMoveParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// カメラコントロールの更新処理
	followCameraController_->Update(context_->inputCommand);
	// プレイヤーの要素をカメラに送る
	followCameraController_->SetFollowPos(player_->GetPlayerPos(), player_->GetVelocity());

	// ボス敵の更新処理
	bossEnemy_->Update();
	bossEnemyModel_->SetDefaultColor({ 1.0f,0.0f,0.0f,bossEnemy_->GetAlpha() });
	// ボスの移動パーティクル
	bossEnmeyMoveParticle_->SetEmitterPos(bossEnemy_->GetPosition());
	bossEnmeyMoveParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// カメラの更新処理
	mainCamera_->SetCamera(followCameraController_->GetCamera());

	airTimer_ += FpsCounter::deltaTime / maxAirTime_;
	if (airTimer_ >= 1.0f) {
		airParticle_->SetFieldAcceleration(Vector3(airSpeed_, -1.0f, 0.0f));
		airSpeed_ *= -1.0f;
		airTimer_ = 0.0f;
	}
	// 空気を演出するためのパーティクル
	airParticle_->Update(mainCamera_->GetWorldMatrix(), mainCamera_->GetViewMatrix());

	// 当たり判定の更新処理
	UpdateCollision();

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

	// UIの更新処理
#pragma region UIUpdate

	// ボスのHpUIの更新処理
	bossHpUI_->SetCurrentHp(bossEnemy_->GetCurrentHp());
	bossHpUI_->Update();

#pragma endregion
}

void ALGameScene::UpdateCollision() {

	// プレイヤーの当たり判定を登録する
	collisionManager_->AddCollider(player_->GetCollider());
	// デバックデータを取得する
	debugRenderer_->AddSphere(player_->GetSphereData());

	// ボス敵の当たり判定を登録する
	collisionManager_->AddCollider(bossEnemy_->GetCollider());
	// デバックデータを取得する
	debugRenderer_->AddSphere(bossEnemy_->GetSphereData());

	// 衝突判定
	collisionManager_->CheckAllCollisions();
}
