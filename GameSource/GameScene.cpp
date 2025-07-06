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
	delete sphereModel_;

	delete neonTextModel_;
	delete neonFrameModel_;
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

	// 平面モデルを生成
	planeModel_ = Model::CreateModel("plane.gltf", "Plane");
	planeGH_ = textureManager->Load("Resources/uvChecker.png");
	planeTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{5.0f,3.0f,0.0f} };
	planeWorldTransform_.Initialize(planeTransform_);

	// 地面モデルを生成
	terrainModel_ = Model::CreateModel("terrain.obj","terrain");
	terrainModel_->SetDefaultIsEnableLight(true);
	grassGH_ = textureManager->Load("Resources/terrain/grass.png");
	terrainWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });
	sphereModel_ = Model::CreateModel("cube.obj", "cube");
	whiteGH_ = textureManager->Load("Resources/white4x4.png");
	// ロープ
	rope_ = std::make_unique<Rope>();
	rope_->Initialize(sphereModel_, whiteGH_);

	// 箱
	boxModel_ = Model::CreateModel("cube.obj", "cube");
	boxTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,2.0f,0.0f} };
	boxWorldTransform_.Initialize(boxTransform_);

	// ネオン
	neonTextTransform_ = { {1.0f,1.0f,1.0f},{0.0f,3.2f,0.0f},{0.0f,2.0f,0.0f} };
	neonTextModel_ = Model::CreateModel("neonText.obj", "NeonText");
	neonTextModel_->SetDefaultColor({ 1.0f,0.0f,1.0f,1.0f });
	neonFrameModel_ = Model::CreateModel("neonFrame.obj", "NeonFrame");
	neonFrameModel_->SetDefaultColor({ 0.0f,0.0f,1.0f,1.0f });
	neonTextWorldTransform_.Initialize(neonTextTransform_);

	// 平行光源ライト
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(dxCommon_->GetDevice(), true, false, false);
	directionalData_.active = true;
	directionalData_.color = { 1.0f,1.0f,1.0f,1.0f };
	directionalData_.direction = { 0.0,1.0f,0.0f };
	directionalData_.intensity = 10.0f;
	lightManager_->SetDirectionalData(directionalData_);

	//boxModel_->SetDefaultColor({1.0f,1.0f,1.0f,1.0f});
}

void GameScene::Update(GameEngine::Input* input){

	// 地面の更新処理
	terrainWorldTransform_.UpdateTransformMatrix();

	// ロープの更新処理
	rope_->Update();

	boxWorldTransform_.UpdateTransformMatrix();
	neonTextWorldTransform_.UpdateTransformMatrix();

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

	//ImGui::ColorEdit3("boxcolor", &color_.x);
	//boxModel_->SetDefaultColor(color_);
	//
	//Vector3 hsv = ColorConverter::RGBtoHSV({color_.x,color_.y,color_.z});
	//ImGui::Text("h:%.3f,s:%.3f,v:%.3f", hsv.x, hsv.y, hsv.z);
		

	// ネオン
	//ImGui::DragFloat3("NeonPos", &neonTextTransform_.translate.x, 0.01f);
	//neonTextWorldTransform_.SetTranslate(neonTextTransform_.translate);

	//ImGui::ColorEdit3("neonColor",&color_.x);
	//neonTextModel_->SetDefaultColor(color_);
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

	// 線の前描画処理
	//PrimitiveRenderer::PreDraw();
	//// 線を描画
	//rope_->DrawLine(camera_->GetVPMatrix());

	// モデルの単体描画前処理
	Model::PreDraw(PSOMode::triangle, blendMode_);
	
	// 箱
	boxModel_->Draw(boxWorldTransform_, whiteGH_, camera_->GetVPMatrix());

	// ネオン
	//neonTextModel_->Draw(neonTextWorldTransform_, whiteGH_, camera_->GetVPMatrix());
	// フレーム
	//neonFrameModel_->Draw(neonTextWorldTransform_, whiteGH_, camera_->GetVPMatrix());

	// 線の点を描画
	//rope_->DrawSphere(camera_->GetVPMatrix());

	// 平面を描画
	planeModel_->Draw(planeWorldTransform_, planeGH_, camera_->GetVPMatrix());

	// 地面を描画
	terrainModel_->DrawLight(lightManager_->GetResource(), camera_->GetCameraResource());
	terrainModel_->Draw(terrainWorldTransform_, grassGH_, camera_->GetVPMatrix());

	// 軸を描画
	//axisIndicator_->Draw(axisTextureHandle_);
}