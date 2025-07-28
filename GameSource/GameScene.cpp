#include"GameScene.h"
#include<numbers>
#include"EngineSource/2D/ImGuiManager.h"
#include"EngineSource/3D/PrimitiveRenderer.h"
#include"EngineSource/Math/MyMath.h"

#include"ColorEditor.h"

using namespace GameEngine;

GameScene::~GameScene() {
	delete planeModel_;
	delete sphereModel_;
	delete UtahTeapotModel_;
	delete bunnyModel_;
	delete suzanneModel_;
	delete multiMeshModel_;
	delete multiMaterialModel_;

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
	// 球モデルを生成
	sphereModel_ = Model::CreateSphere(16);
	sphereModel_->SetDefaultIsEnableLight(true);
	// ティーポッドモデルを生成
	UtahTeapotModel_ = Model::CreateModel("teapot.obj", "Teapot");
	UtahTeapotModel_->SetDefaultIsEnableLight(true);
	// ウサギモデルを生成
	bunnyModel_ = Model::CreateModel("bunny.obj", "Bunny");
	bunnyModel_->SetDefaultIsEnableLight(true);
	// スザンヌモデルを生成
	suzanneModel_ = Model::CreateModel("suzanne.obj", "Suzanne");
	suzanneModel_->SetDefaultIsEnableLight(true);
	// マルチメッシュを生成
	multiMeshModel_ = Model::CreateModel("multiMesh.obj", "MultiMesh");
	multiMeshModel_->SetDefaultIsEnableLight(true);
	// マルチマテリアル
	multiMaterialModel_ = Model::CreateModel("multiMaterial.obj", "MultiMaterial");
	multiMaterialModel_->SetDefaultIsEnableLight(true,0);
	multiMaterialModel_->SetDefaultIsEnableLight(true,1);

	// 評価課題のモデルを描画するクラスの初期化
	drawTaskModels_ = std::make_unique<DrawTaskModel>();
	drawTaskModels_->Initialize(uvCheckerGH_, whiteGH_);
	// 平面モデルをセット
	drawTaskModels_->SetPlane(planeModel_);
	// 球モデルをセット
	drawTaskModels_->SetSphere(sphereModel_);
	// ティーポッドモデルをセット
	drawTaskModels_->SetUtahTeapot(UtahTeapotModel_);
	// ウサギモデルをセット
	drawTaskModels_->SetBunny(bunnyModel_);
	// スザンヌモデルをセット
	drawTaskModels_->SetSuzanne(suzanneModel_);
	// マルチメッシュをセット
	drawTaskModels_->SetMultiMesh(multiMeshModel_);
	// マルチマテリアル
	drawTaskModels_->SetMultiMaterial(multiMaterialModel_);
}

void GameScene::Update(GameEngine::Input* input){

	// Fps計測器の更新処理
	fpsCounter_->Update();

	// 地面の更新処理
	terrainWorldTransform_.UpdateTransformMatrix();

	// 更新
	drawTaskModels_->Update();

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

	// モデルのデバック
	drawTaskModels_->DebugWindow();

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
	if(ImGui::Button("PlayerSound")) {
		audioManager_->Play(seHandle_);
	}
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

	// スプライトの描画前処理
	Sprite::PreDraw(BlendMode::kBlendModeNormal);

	// 描画
	drawTaskModels_->Draw2D();

	// モデルの単体描画前処理
	Model::PreDraw(PSOMode::Triangle, BlendMode::kBlendModeNormal);

	// 地面を描画
	//terrainModel_->DrawLight(lightManager_->GetResource(), camera_->GetCameraResource());
	//terrainModel_->Draw(terrainWorldTransform_, grassGH_, camera_->GetVPMatrix());

	// 描画
	drawTaskModels_->Draw3D(camera_->GetVPMatrix(), lightManager_->GetResource(), camera_->GetCameraResource());

	// モデルの単体描画前処理
	Model::PreDraw(PSOMode::Grid, BlendMode::kBlendModeNormal);

	// グリッドを描画
	gridModel_->DrawGrid(gridWorldTransform_, camera_->GetVPMatrix(), debugCamera_->GetCameraResource());

	// 軸を描画
	//axisIndicator_->Draw(axisTextureHandle_);
}