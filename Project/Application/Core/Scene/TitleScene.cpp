#include"TitleScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
using namespace GameEngine;

TitleScene::~TitleScene() {
}

void TitleScene::Initialize(GameEngine::Input* input, GameEngine::InputCommand* inputCommand, GameEngine::ModelManager* modelManager, GameEngine::TextureManager* textureManager, GameEngine::AudioManager* audioManager, GameEngine::DirectXCommon* dxCommon) {
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
	gridModel_ = modelManager->GetNameByModel("Grid");
	gridWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
#pragma endregion
	
	inputCommand_->RegisterCommand("CameraChange", {{InputState::KeyTrigger, DIK_F }});
}

void TitleScene::Update() {

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
		camera_->Update();

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

void TitleScene::Draw() {

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	//ModelRenderer::PreDraw(RenderMode::DefaultModel, BlendMode::kBlendModeNormal);


#ifdef _DEBUG
	// モデルの単体描画前処理
	ModelRenderer::PreDraw(RenderMode::Grid, BlendMode::kBlendModeNone);

	// グリッドを描画
	ModelRenderer::DrawGrid(gridModel_, gridWorldTransform_, camera_->GetVPMatrix(), debugCamera_->GetCameraResource());
#endif
}
