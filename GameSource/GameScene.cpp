#include"GameScene.h"
#include<numbers>
#include"EngineSource/2D/ImGuiManager.h"
#include"EngineSource/Math/MyMath.h"

using namespace GameEngine;

GameScene::~GameScene() {
	delete shereModel_;
	delete terrainModel_;
}

void GameScene::Initialize(GameEngine::TextureManager* textureManager, GameEngine::DirectXCommon* dxCommon) {

	// DirectX機能を受け取る
	dxCommon_ = dxCommon;

	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize(cameraTransform_, 1280, 720,dxCommon_->GetDevice());
	// デバックカメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize({ 0.0f,2.0f,-20.0f }, 1280, 720, dxCommon_->GetDevice());

	// 軸方向表示の初期化
	axisIndicator_ = std::make_unique<AxisIndicator>();
	axisIndicator_->Initialize("axis.obj");
	// 軸方向表示の画像
	axisTextureHandle_ = textureManager->Load("Resources/axis/axis.jpg");

	// 球モデルを生成
	shereModel_ = Model::CreateSphere(16);
	// モンスターボールのテクスチャを生成
	monsterBallGH_ = textureManager->Load("Resources/monsterBall.png");
	// 球のトランスフォームを生成
	shereWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });

	// 地面モデルを生成
	terrainModel_ = Model::CreateFromOBJ("terrain.obj","terrain");
	grassGH_ = textureManager->Load("Resources/terrain/grass.png");
	terrainWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });
}

void GameScene::Update(GameEngine::Input* input){

	shereWorldTransform_.UpdateTransformMatrix();

	terrainWorldTransform_.UpdateTransformMatrix();

	// カメラ処理
	if (isDebugCameraActive_) {
		// デバックカメラの更新
		debugCamera_->Update(input);
		// デバックカメラの値をカメラに代入
		camera_->SetVPMatrix(debugCamera_->GetVPMatrix());
		// 軸方向表示の更新処理
		axisIndicator_->Update(debugCamera_->GetRotateMatrix());
	} else {
		// カメラの更新処理
		//camera_->Update();
		camera_->SetCameraPosition(cameraTransform_);
	}

#ifdef _DEBUG
	// 光源をデバック
	ImGui::Begin("DebugWindow");
	// カメラのトラスフォーム
	ImGui::DragFloat3("CameraPos", &cameraTransform_.translate.x, 0.01f);
	ImGui::DragFloat3("CameraRotate", &cameraTransform_.rotate.x, 0.01f);
	ImGui::DragFloat3("CameraScale", &cameraTransform_.scale.x, 0.01f);
	// ブレンドモードの切り替え
	if (ImGui::Combo("BlendMode", &selectBlendNum_, blendModeName_, IM_ARRAYSIZE(blendModeName_))) {
		if (selectBlendNum_ == BlendMode::kBlendModeNone) {
			blendMode_ = BlendMode::kBlendModeNone;
		} else if (selectBlendNum_ == BlendMode::kBlendModeNormal) {
			blendMode_ = BlendMode::kBlendModeNormal;
		} else if (selectBlendNum_ == BlendMode::kBlendModeAdd) {
			blendMode_ = BlendMode::kBlendModeAdd;
		} else if (selectBlendNum_ == BlendMode::kBlendModeSubtract) {
			blendMode_ = BlendMode::kBlendModeSubtract;
		} else if (selectBlendNum_ == BlendMode::kBlendModeMultily) {
			blendMode_ = BlendMode::kBlendModeMultily;
		} else if (selectBlendNum_ == BlendMode::kBlendModeScreen) {
			blendMode_ = BlendMode::kBlendModeScreen;
		}
	}
	ImGui::End();

	// カメラの切り替え処理
	if (input->TriggerKey(DIK_SPACE)) {
		if (isDebugCameraActive_) {
			isDebugCameraActive_ = false;
		} else {
			isDebugCameraActive_ = true;
		}
	}
#endif 
}

void GameScene::Draw() {

	// モデルの単体描画前処理
	Model::PreDraw(PSOMode::triangle, BlendMode::kBlendModeNone);

	// 地面を描画
	terrainModel_->Draw(terrainWorldTransform_, grassGH_, camera_->GetVPMatrix());

	// 球を描画
	shereModel_->Draw(shereWorldTransform_, monsterBallGH_, camera_->GetVPMatrix());

	// 軸を描画
	axisIndicator_->Draw(axisTextureHandle_);
}