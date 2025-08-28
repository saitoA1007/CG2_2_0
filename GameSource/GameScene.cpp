#include"GameScene.h"
#include<numbers>
#include"EngineSource/2D/ImGuiManager.h"
#include"EngineSource/3D/PrimitiveRenderer.h"
#include"EngineSource/Math/MyMath.h"

#include"ColorEditor.h"

using namespace GameEngine;

GameScene::~GameScene() {
	delete planeModel_;

	delete terrainModel_;

	// グリッドの解放
	delete gridModel_;
}

void GameScene::Initialize(GameEngine::TextureManager* textureManager, GameEngine::DirectXCommon* dxCommon, GameEngine::AudioManager* audioManager) {

	// DirectX機能を受け取る
	dxCommon_ = dxCommon;

	// 音声機能を受け取る
	audioManager_ = audioManager;
	// ロード
	seHandle_ = audioManager_->Load("Resources/Sounds/se.mp3");

	// テクスチャ機能を受け取る
	textureManager_ = textureManager;

	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize(cameraTransform_, 1280, 720,dxCommon_->GetDevice());
	// デバックカメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize({ 0.0f,2.0f,-20.0f }, 1280, 720, dxCommon_->GetDevice());

	// 軸方向表示の初期化
	axisIndicator_ = std::make_unique<AxisIndicator>();
	axisIndicator_->Initialize();

	// fps計測器の初期化
	fpsCounter_ = std::make_unique<FpsCounter>();
	fpsCounter_->Initialize();

	// white4x4テクスチャをロード
	whiteGH_ = textureManager->Load("Resources/Textures/white4x4.png");

	// グリッドの初期化
	gridModel_ = Model::CreateGridPlane({200.0f,200.0f});
	gridWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// 地面モデルを生成
	terrainModel_ = Model::CreateModel("terrain.obj","Terrain");
	terrainModel_->SetDefaultIsEnableLight(true);
	grassGH_ = textureManager->Load("Resources/Models/Terrain/grass.png");
	terrainWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });

	// 平行光源ライト
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(dxCommon_->GetDevice(), true, false, false);
	directionalData_.active = true;
	directionalData_.color = { 1.0f,1.0f,1.0f,1.0f };
	directionalData_.direction = { 0.0,0.0f,1.0f };
	directionalData_.intensity = 1.0f;
	lightManager_->SetDirectionalData(directionalData_);

	// 平面モデルを生成
	planeModel_ = Model::CreateModel("plane.obj", "Plane");
	planeModel_->SetDefaultIsEnableLight(true);
	uvCheckerGH_ = textureManager->Load("Resources/Textures/uvChecker.png");
}

void GameScene::Update(GameEngine::Input* input){

	// Fps計測器の更新処理
	fpsCounter_->Update();

	// 地面の更新処理
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

	// ライトの更新
	lightManager_->Update();

	// グリッドの更新処理
	gridWorldTransform_.SetTranslate(Vector3(debugCamera_->GetTargetPosition().x,-0.1f, debugCamera_->GetTargetPosition().z));
	gridWorldTransform_.UpdateTransformMatrix();

	// 光源をデバック
	ImGui::Begin("DebugWindow");

	// 平行光源
	if (ImGui::TreeNodeEx("Light", ImGuiTreeNodeFlags_Framed)) {
		ImGui::DragFloat3("Dirction", &directionalData_.direction.x, 0.01f);
		directionalData_.direction = Normalize(directionalData_.direction);
		ImGui::DragFloat("Intensity", &directionalData_.intensity, 0.01f);
		ImGui::ColorEdit3("Color", &directionalData_.color.x);
		lightManager_->SetDirectionalData(directionalData_);
		ImGui::TreePop();
	}
	ImGui::End();

	ImGui::Begin("AudioManager");
	// ボタンを押したらseを再生
	if(ImGui::Button("PlaySound")) {
		audioManager_->Play(seHandle_);
	}

	ImGui::End();

	// Fps計測器の描画
	fpsCounter_->DrawImGui();

	ImGui::Begin("Gizmo Example");

	// 3. ImGuizmo 設定（画像の上に描く！）
	//ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

	// 4. ギズモ表示（カメラ行列と一致させる）
	float view[16];       // → GPUと同じビュー行列
	float proj[16]; // → GPUと同じプロジェクション行列
	float model[16];      // → オブジェクトの現在の行列

	// コピー
	std::memcpy(view, camera_->GetViewMatrix().m, sizeof(float) * 16);
	std::memcpy(proj, camera_->GetProjectionMatrix().m, sizeof(float) * 16);
	std::memcpy(model, terrainWorldTransform_.GetWorldMatrix().m, sizeof(float) * 16);

	ImGuizmo::Manipulate(view, proj, ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, model);

	ImGui::End();

	// カメラの切り替え処理
	if (input->TriggerKey(DIK_SPACE)) {
		if (isDebugCameraActive_) {
			isDebugCameraActive_ = false;
		} else {
			isDebugCameraActive_ = true;
		}
	}
}

void GameScene::Draw() {

	// モデルの単体描画前処理
	Model::PreDraw(PSOMode::Triangle, BlendMode::kBlendModeNormal);

	// 地面を描画
	terrainModel_->DrawLight(lightManager_->GetResource(), camera_->GetCameraResource());
	terrainModel_->Draw(terrainWorldTransform_, grassGH_, camera_->GetVPMatrix());

	// モデルの単体描画前処理
	Model::PreDraw(PSOMode::Grid, BlendMode::kBlendModeNormal);

	// グリッドを描画
	gridModel_->DrawGrid(gridWorldTransform_, camera_->GetVPMatrix(), debugCamera_->GetCameraResource());

	// 軸を描画
	//axisIndicator_->);
}