#include"TDGameScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"SpriteRenderer.h"
#include"GameParamEditor.h"
#include"LogManager.h"
#include<numbers>
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
	terrainModel_ = context_->modelManager->GetNameByModel("PlaneXZ");
	grassGH_ = context_->textureManager->GetHandleByName("grass.png");
	terrainWorldTransform_.Initialize({ {30.0f,30.0f,30.0f},{0.0f,0.0f,0.0f},{0.0f,-0.2f,0.0f} });

	// ライトの生成
	sceneLightingController_ = std::make_unique<SceneLightingController>();
	sceneLightingController_->Initialize(context_->graphicsDevice->GetDevice());

	// プレイヤーモデルを生成
	playerModel_ = context_->modelManager->GetNameByModel("Triangular");
	playerModel_->SetDefaultIsEnableLight(true);
	// プレイヤークラスを初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();

	// カメラをコントロールするクラスを初期化
	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize();

	// 入力コマンドを設定する
	InputRegisterCommand();
}

void TDGameScene::Update() {

	// デバックリストを削除
	debugRenderer_->Clear();

	// ライトの更新処理
	sceneLightingController_->Update();

	// プレイヤーの更新処理
	player_->Update(context_->inputCommand);

	// カメラコントロールの更新処理
	cameraController_->Update(context_->inputCommand, player_->GetPlayerPos());

	// カメラの更新処理
	mainCamera_->SetCamera(cameraController_->GetCamera());

	// 当たり判定の更新処理
	UpdateCollision();
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

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// 天球の描画
	ModelRenderer::Draw(skyDomeModel_, skyDomeWorldTransform_);

	// 地面を描画
	ModelRenderer::Draw(terrainModel_, terrainWorldTransform_, grassGH_);

	// プレイヤーを描画
	uint32_t DefaultWhiteGH = 0;
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(playerModel_, player_->GetWorldTransform(), DefaultWhiteGH);

#ifdef _DEBUG

	// デバック描画
	if (isDebugView) {
		debugRenderer_->DrawAll(context_->debugCamera_->GetVPMatrix());
	}
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

	// カメラ操作のコマンドを登録する
	context_->inputCommand->RegisterCommand("CameraMoveLeft", { { InputState::KeyPush, DIK_LEFT },{InputState::PadRightStick,0,{-1.0f,0.0f},0.2f} });
	context_->inputCommand->RegisterCommand("CameraMoveRight", { { InputState::KeyPush, DIK_RIGHT },{InputState::PadRightStick,0,{1.0f,0.0f},0.2f} });
}

void TDGameScene::UpdateCollision() {


	// 衝突判定
	collisionManager_->CheckAllCollisions();
}
