#include"GameScene.h"
#include"EngineSource/2D/ImGuiManager.h"
#include"EngineSource/Math/MyMath.h"

using namespace GameEngine;

GameScene::~GameScene() {
	// 平面モデルを解放
	delete planeModel_;
}

void GameScene::Initialize(GameEngine::TextureManager* textureManager, GameEngine::DirectXCommon* dxCommon) {

	// DirectX機能を受け取る
	dxCommon_ = dxCommon;

	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720);
	// デバックカメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize({ 0.0f,0.0f,-10.0f }, 1280, 720);

	// 軸方向表示の初期化
	axisIndicator_ = std::make_unique<AxisIndicator>();
	axisIndicator_->Initialize("axis.obj");
	// 軸方向表示の画像
	axisTextureHandle_ = textureManager->Load("Resources/axis/axis.jpg");

	// 平行光源の生成
	lightColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	lightDir_ = { 0.1f,-0.9f,0.1f };
	intensity_ = 6.2f;
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize(dxCommon->GetDevice(), lightColor_, lightDir_, intensity_);

	// テクスチャを生成
	uvTextureHandle_ = textureManager->Load("Resources/uvChecker.png");
	// 平面モデルを生成
	planeModel_ = Model::CreateFromOBJ("plane.obj", "Plane");
	// ワールド行列を更新
	planeWorldTransform_.Initialize({ { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } });
}

void GameScene::Update(GameEngine::Input* input) {

	// 平面の色を設定
	planeModel_->SetDefaultColor(planeColor_);

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
		camera_->Update();
	}

#ifdef _DEBUG
	// 光源をデバック
	ImGui::Begin("DebugWindow");
	ImGui::ColorEdit4("planeColor", &planeColor_.x);
	// 光の色を変更
	ImGui::ColorEdit3("LightColor", &lightColor_.x);
	directionalLight_->SetLightColor(lightColor_);
	// 光の方向を変更
	ImGui::SliderFloat3("LightDirection", &lightDir_.x, -1.0f, 1.0f);
	lightDir_ = Normalize(lightDir_);
	directionalLight_->SetLightDir(lightDir_);
	// 光の強度を変更
	ImGui::SliderFloat("LightIntensity", &intensity_, 0.0f, 10.0f);
	directionalLight_->SetLightIntensity(intensity_);
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

	// モデルの描画前処理
	Model::PreDraw();

	// 平面モデルを描画
	planeModel_->Draw(planeWorldTransform_, uvTextureHandle_, camera_->GetVPMatrix());

	// 軸を描画
	axisIndicator_->Draw(axisTextureHandle_);
}