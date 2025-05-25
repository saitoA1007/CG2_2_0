#include"GameScene.h"
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
	debugCamera_->Initialize({ 0.0f,0.0f,-10.0f }, 1280, 720, dxCommon_->GetDevice());

	// 軸方向表示の初期化
	axisIndicator_ = std::make_unique<AxisIndicator>();
	axisIndicator_->Initialize("axis.obj");
	// 軸方向表示の画像
	axisTextureHandle_ = textureManager->Load("Resources/axis/axis.jpg");

	// 平行光源の生成
	lightColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	lightDir_ = { 0.1f,-0.9f,0.1f };
	intensity_ = 1.0f;
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize(dxCommon->GetDevice(), lightColor_, lightDir_, intensity_);

	// 点光源ライト
	pointLightColor_ = {1.0f,1.0f,1.0f,1.0f};
	pointLightPos_ = { 0.0f,0.0f,-5.0f };
	pointLightIntensity_ = 1.0f;
	pointLight_ = std::make_unique<PointLight>();
	pointLight_->Initialize(dxCommon->GetDevice(), pointLightColor_, pointLightPos_, pointLightIntensity_);

	// 球モデルを生成
	shereModel_ = Model::CreateSphere(16);
	shereModel_->SetDefaultIsEnableLight(true);
	// モンスターボールのテクスチャを生成
	monsterBallGH_ = textureManager->Load("Resources/monsterBall.png");
	// 球のトランスフォームを生成
	shereWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });

	// 地面モデルを生成
	terrainModel_ = Model::CreateFromOBJ("terrain.obj","terrain");
	terrainModel_->SetDefaultIsEnableLight(true);
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
	// 光の色を変更
	ImGui::ColorEdit3("PointColor", &pointLightColor_.x);
	pointLight_->SetLightColor(pointLightColor_);
	// 光の位置を変更
	ImGui::SliderFloat3("PointLightPos", &pointLightPos_.x, -10.0f, 10.0f);
	pointLight_->SetLightPosition(pointLightPos_);
	// 光の強度を変更
	ImGui::SliderFloat("PointLightIntensity", &pointLightIntensity_, 0.0f, 10.0f);
	pointLight_->SetLightIntensity(pointLightIntensity_);
	// 光の範囲
	ImGui::SliderFloat("PointLightRadiuse", &radius_, 0.0f, 10.0f);
	pointLight_->SetRadius(radius_);
	// 光の減衰率
	ImGui::SliderFloat("PointLightDecay", &decay_, 0.0f, 10.0f);
	pointLight_->SetDecay(decay_);
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
	terrainModel_->DrawLight(directionalLight_->GetResource(), camera_->GetCameraResource(), pointLight_->GetResource());
	terrainModel_->Draw(terrainWorldTransform_, grassGH_, camera_->GetVPMatrix());

	// 球を描画
	shereModel_->DrawLight(directionalLight_->GetResource(), camera_->GetCameraResource(), pointLight_->GetResource());
	shereModel_->Draw(shereWorldTransform_, monsterBallGH_, camera_->GetVPMatrix());

	// 軸を描画
	axisIndicator_->Draw(axisTextureHandle_);
}