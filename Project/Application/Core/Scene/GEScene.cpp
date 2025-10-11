#include"GEScene.h"
#include"ImguiManager.h"
#include<numbers>
using namespace GameEngine;

GEScene::~GEScene() {
}

void GEScene::Initialize(GameEngine::Input* input, GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager, GameEngine::AudioManager* audioManager, GameEngine::DirectXCommon* dxCommon) {
	// ゲームシーンに必要な低レイヤー機能
#pragma region SceneSystem 
	// 入力を取得
	input_ = input;
	// テクスチャ機能を取得
	textureManager_ = textureManager;
	// 音声機能を取得
	audioManager_ = audioManager;
	// DirectX機能を取得
	dxCommon_ = dxCommon;
	// 入力処理のコマンドシステムを取得
	inputCommand_ = inputCommand;

	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720);
	// デバックカメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize({ 0.0f,2.0f,-20.0f }, 1280, 720, dxCommon->GetDevice());

	// グリッドの初期化
	gridModel_ = Model::CreateGridPlane({ 200.0f,200.0f });
	gridWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
#pragma endregion

	// プレイヤーモデルを生成
	playerModel_ = Model::CreateModel("cube.obj", "Cube");
	// プレイヤークラスを初期化
	player_ = std::make_unique<Player>();
	player_->Initialize(playerModel_.get());

	// 地面モデルを生成
	terrainModel_ = Model::CreatePlane({ 20.0f,20.0f });
	terrainModel_->SetDefaultColor({ 0.4f,0.4f,0.4f,1.0f });
	terrainWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{std::numbers::pi_v<float> / 2.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// カメラをコントロールするクラスを初期化
	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize();

	// 入力コマンドを設定する
	InputRegisterCommand();

}

void GEScene::Update() {

	// プレイヤーの更新処理
	player_->Update(inputCommand_);

	// カメラコントロールの更新処理
	cameraController_->Update(inputCommand_,player_->GetPlayerPos());

	// カメラ処理
#pragma region Camera
	if (isDebugCameraActive_) {
		// デバックカメラの更新
		debugCamera_->Update(input_);
		// デバックカメラの値をカメラに代入
		camera_->SetVPMatrix(debugCamera_->GetVPMatrix());

		// グリッドの更新処理
		gridWorldTransform_.transform_.translate = Vector3(debugCamera_->GetTargetPosition().x, -0.1f, debugCamera_->GetTargetPosition().z);
		gridWorldTransform_.UpdateTransformMatrix();

	} else {
		// カメラの更新処理
		//camera_->Update();
		camera_->SetVPMatrix(cameraController_->GetVPMatirx());

		// グリッドの更新処理
		gridWorldTransform_.transform_.translate = Vector3(camera_->GetWorldPosition().x, -0.1f, camera_->GetWorldPosition().z);
		gridWorldTransform_.UpdateTransformMatrix();
	}
#pragma endregion

#ifdef _DEBUG

	// カメラの切り替え処理
#pragma region CameraTransition
	if (inputCommand_->IsCommandAcitve("CameraChange")) {
		if (isDebugCameraActive_) {
			isDebugCameraActive_ = false;
		} else {
			isDebugCameraActive_ = true;
		}
	}
#pragma endregion
#endif
}

void GEScene::Draw() {

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	Model::PreDraw(PSOMode::Triangle, BlendMode::kBlendModeNormal);

	// プレイヤーを描画
	player_->Draw(camera_->GetVPMatrix());

	// 地面を描画
	terrainModel_->Draw(terrainWorldTransform_, 0, camera_->GetVPMatrix());

#ifdef _DEBUG
	// モデルの単体描画前処理
	Model::PreDraw(PSOMode::Grid, BlendMode::kBlendModeNormal);

	// グリッドを描画
	gridModel_->DrawGrid(gridWorldTransform_, camera_->GetVPMatrix(), debugCamera_->GetCameraResource());
#endif
}

void GEScene::InputRegisterCommand() {
	// カメラを操作を切り替える入力コマンドを登録
	inputCommand_->RegisterCommand("CameraChange", { {InputState::KeyTrigger, DIK_F },{ InputState::KeyTrigger, DIK_G } });

	// 移動の入力コマンドを登録する
	inputCommand_->RegisterCommand("MoveUp", { {InputState::KeyPush, DIK_W },{InputState::PadLeftStick,0,{0.0f,1.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_UP } });
	inputCommand_->RegisterCommand("MoveDown", { {InputState::KeyPush, DIK_S },{InputState::PadLeftStick,0,{0.0f,-1.0f},0.2f}, {InputState::PadPush, XINPUT_GAMEPAD_DPAD_DOWN} });
	inputCommand_->RegisterCommand("MoveLeft", { {InputState::KeyPush, DIK_A },{InputState::PadLeftStick,0,{-1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_LEFT } });
	inputCommand_->RegisterCommand("MoveRight", { {InputState::KeyPush, DIK_D },{InputState::PadLeftStick,0,{1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_RIGHT } });
	// ジャンプコマンドを登録する
	inputCommand_->RegisterCommand("Jump", { {InputState::KeyTrigger, DIK_SPACE},{InputState::PadTrigger, XINPUT_GAMEPAD_A} });

	// カメラ操作のコマンドを登録する
	inputCommand_->RegisterCommand("CameraMoveLeft", { { InputState::KeyPush, DIK_LEFT },{InputState::PadRightStick,0,{-1.0f,0.0f},0.2f} });
	inputCommand_->RegisterCommand("CameraMoveRight", { { InputState::KeyPush, DIK_RIGHT },{InputState::PadRightStick,0,{1.0f,0.0f},0.2f} });
}
