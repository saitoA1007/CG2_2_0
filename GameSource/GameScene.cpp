#include"GameScene.h"
#include<numbers>
#include"EngineSource/2D/ImGuiManager.h"
#include"EngineSource/3D/PrimitiveRenderer.h"
#include"EngineSource/Math/MyMath.h"

#include"ColorEditor.h"

using namespace GameEngine;

GameScene::~GameScene() {
	delete planeModel_;
	delete boxModel_;
	delete terrainModel_;

	// グリッドの解放
	delete gridModel_;
}

void GameScene::Initialize(GameEngine::TextureManager* textureManager, GameEngine::DirectXCommon* dxCommon) {

	// DirectX機能を受け取る
	dxCommon_ = dxCommon;

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
	// 軸方向表示の画像
	axisTextureHandle_ = textureManager->Load("Resources/Models/Axis/axis.jpg");

	// fps計測器の初期化
	fpsCounter_ = std::make_unique<FpsCounter>();
	fpsCounter_->Initialize();

	// white4x4テクスチャをロード
	whiteGH_ = textureManager->Load("Resources/Textures/white4x4.png");

	// グリッドの初期化
	gridModel_ = Model::CreateGridPlane({200.0f,200.0f});
	gridWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// 平面モデルを生成
	planeModel_ = Model::CreateModel("plane.gltf", "Plane");
	planeGH_ = textureManager->Load("Resources/Textures/uvChecker.png");
	planeTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{5.0f,3.0f,0.0f} };
	planeWorldTransform_.Initialize(planeTransform_);

	// 地面モデルを生成
	terrainModel_ = Model::CreateModel("terrain.obj","Terrain");
	terrainModel_->SetDefaultIsEnableLight(true);
	grassGH_ = textureManager->Load("Resources/Models/Terrain/grass.png");
	terrainWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });

	// 箱
	boxModel_ = Model::CreateModel("cube.obj", "Cube");
	boxTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,2.0f,0.0f} };
	boxWorldTransform_.Initialize(boxTransform_);

	// 平行光源ライト
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(dxCommon_->GetDevice(), true, false, false);
	directionalData_.active = true;
	directionalData_.color = { 1.0f,1.0f,1.0f,1.0f };
	directionalData_.direction = { 0.0,1.0f,0.0f };
	directionalData_.intensity = 10.0f;
	lightManager_->SetDirectionalData(directionalData_);
}

void GameScene::Update(GameEngine::Input* input){

	// Fps計測器の更新処理
	fpsCounter_->Update();

	// 地面の更新処理
	terrainWorldTransform_.UpdateTransformMatrix();

	boxWorldTransform_.UpdateTransformMatrix();

	// 平面の更新処理
	planeWorldTransform_.SetTransform(planeTransform_);
	planeWorldTransform_.UpdateTransformMatrix();

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

#ifdef _DEBUG
	// 光源をデバック
	ImGui::Begin("DebugWindow");
	// カメラのトラスフォーム
	ImGui::DragFloat3("CameraPos", &cameraTransform_.translate.x, 0.01f);
	ImGui::DragFloat3("CameraRotate", &cameraTransform_.rotate.x, 0.01f);
	ImGui::DragFloat3("CameraScale", &cameraTransform_.scale.x, 0.01f);
	// ブレンドモードの切り替え
	if (ImGui::Combo("BlendMode", &selectBlendNum_, blendModeName_, IM_ARRAYSIZE(blendModeName_))) {
		switch (selectBlendNum_) {
		case BlendMode::kBlendModeNone:
			blendMode_ = BlendMode::kBlendModeNone;
			break;
		case BlendMode::kBlendModeNormal:
			blendMode_ = BlendMode::kBlendModeNormal;
			break;
		case BlendMode::kBlendModeAdd:
			blendMode_ = BlendMode::kBlendModeAdd;
			break;
		case BlendMode::kBlendModeSubtract:
			blendMode_ = BlendMode::kBlendModeSubtract;
			break;
		case BlendMode::kBlendModeMultily:
			blendMode_ = BlendMode::kBlendModeMultily;
			break;
		case BlendMode::kBlendModeScreen:
			blendMode_ = BlendMode::kBlendModeScreen;
			break;
		}
	}
	// 平行光源
	if (ImGui::TreeNode("DirectionalLight")) {
		ImGui::DragFloat3("dir", &directionalData_.direction.x, 0.01f);
		directionalData_.direction = Normalize(directionalData_.direction);
		ImGui::DragFloat("ind", &directionalData_.intensity, 0.01f);
		lightManager_->SetDirectionalData(directionalData_);
		ImGui::TreePop();
	}
	// 平面
	if (ImGui::TreeNode("plane")) {
		ImGui::DragFloat3("scale", &planeTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("rotate", &planeTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("translate", &planeTransform_.translate.x, 0.01f);
		ImGui::TreePop();
	}

	ImGui::DragFloat3("boxPos", &boxTransform_.translate.x, 0.01f);
	boxWorldTransform_.SetTranslate(boxTransform_.translate);

	ImGui::End();

	// Fps計測器の描画
	fpsCounter_->DrawImGui();

#endif

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
	Model::PreDraw(PSOMode::Triangle, blendMode_);
	
	// 箱
	boxModel_->Draw(boxWorldTransform_, whiteGH_, camera_->GetVPMatrix());

	// 平面を描画
	planeModel_->Draw(planeWorldTransform_, planeGH_, camera_->GetVPMatrix());

	// 地面を描画
	terrainModel_->DrawLight(lightManager_->GetResource(), camera_->GetCameraResource());
	terrainModel_->Draw(terrainWorldTransform_, grassGH_, camera_->GetVPMatrix());

	// モデルの単体描画前処理
	Model::PreDraw(PSOMode::Grid, blendMode_);

	// グリッドを描画
	gridModel_->DrawGrid(gridWorldTransform_, camera_->GetVPMatrix(), debugCamera_->GetCameraResource());

	// 軸を描画
	//axisIndicator_->Draw(axisTextureHandle_);
}