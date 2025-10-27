#include"GameScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"

#include"Application/GameParamEditor.h"
#include"FPSCounter.h"

using namespace GameEngine;

GameScene::~GameScene() {

}

void GameScene::Initialize(GameEngine::Input* input, GameEngine::InputCommand* inputCommand, GameEngine::ModelManager* modelManager, GameEngine::TextureManager* textureManager, GameEngine::AudioManager* audioManager, GameEngine::DirectXCommon* dxCommon) {
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
	camera_->Initialize(cameraTransform_, 1280, 720, dxCommon_->GetDevice());
	// デバックカメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize({ 0.0f,2.0f,-20.0f }, 1280, 720, dxCommon_->GetDevice());

	// グリッドの初期化
	gridModel_ = modelManager->GetNameByModel("Grid");
	gridWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
#pragma endregion

	// 平行光源ライト
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(dxCommon_->GetDevice(), true, false, false);
	directionalData_.active = true;
	directionalData_.color = { 1.0f,1.0f,1.0f,1.0f };
	directionalData_.direction = { 0.0,0.0f,1.0f };
	directionalData_.intensity = 1.0f;
	lightManager_->SetDirectionalData(directionalData_);

	// 地面モデルを生成
	terrainModel_ = modelManager->GetNameByModel("terrain.obj");
	terrainModel_->SetDefaultIsEnableLight(true);
	grassGH_ = textureManager->GetHandleByName("grass");
	terrainWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });

	// 平面モデルを生成
	planeModel_ = modelManager->GetNameByModel("plane.obj");
	planeModel_->SetDefaultIsEnableLight(true);
	uvCheckerGH_ = textureManager->GetHandleByName("uvChecker");
	planeWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f} });

	// ボーンアニメーションを生成する
	bronAnimationModel_ = modelManager->GetNameByModel("walk.gltf");
	bronAnimationWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
	// ボーンアニメーションデータを取得する
	bronAnimation_ = Model::LoadAnimationFile("walk.gltf", "Walk");
	skeletonBron_ = Model::CreateSkeleton(bronAnimationModel_->modelData_.rootNode);
	skinClusterBron_ = Animation::CreateSkinCluster(skeletonBron_, bronAnimationModel_->modelData_);
	timer_ = 0.0f;

	RegisterBebugParam();
	ApplyDebugParam();
}

void GameScene::Update() {

	ApplyDebugParam();

	// 地面の更新処理
	terrainWorldTransform_.UpdateTransformMatrix();

	// ライトの更新
	lightManager_->Update();

	timer_ += FpsCounter::deltaTime;
	float animationTime = fmodf(timer_, bronAnimation_.duration);
	Animation::Update(skinClusterBron_, skeletonBron_, bronAnimation_, animationTime);

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

	// 光源をデバック
	ImGui::Begin("DebugWindow");
	// カメラのデバック
	ImGui::DragFloat3("CameraTranslate", &cameraTransform_.translate.x, 0.01f);
	ImGui::DragFloat3("CameraRotate", &cameraTransform_.rotate.x, 0.01f);

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

	// カメラの切り替え処理
#pragma region CameraTransition
	if (input_->TriggerKey(DIK_F)) {
		if (isDebugCameraActive_) {
			isDebugCameraActive_ = false;
		} else {
			isDebugCameraActive_ = true;
		}
	}
#pragma endregion

#endif
}

void GameScene::Draw() {

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode::DefaultModel, BlendMode::kBlendModeNormal);

	// 地面を描画
	ModelRenderer::DrawLight(lightManager_->GetResource(), camera_->GetCameraResource());
	ModelRenderer::Draw(terrainModel_, terrainWorldTransform_, grassGH_, camera_->GetVPMatrix());

	// 平面描画
	//planeModel_->Draw(planeWorldTransform_, uvCheckerGH_, camera_->GetVPMatrix());

	// アニメーションの描画前処理
	ModelRenderer::PreDraw(RenderMode::AnimationModel, BlendMode::kBlendModeNone);

	// アニメーションしているモデルを描画
	ModelRenderer::DrawAnimation(bronAnimationModel_, bronAnimationWorldTransform_, camera_->GetVPMatrix(), skinClusterBron_);

#ifdef _DEBUG
	// モデルの単体描画前処理
	ModelRenderer::PreDraw(RenderMode::Grid, BlendMode::kBlendModeNone);

	// グリッドを描画
	ModelRenderer::DrawGrid(gridModel_, gridWorldTransform_, camera_->GetVPMatrix(), debugCamera_->GetCameraResource());
#endif
}

void GameScene::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem("Test1", "testNum", testNumber);
	GameParamEditor::GetInstance()->AddItem("Test2", "testVec", testVector);
}

void GameScene::ApplyDebugParam() {
	testNumber = GameParamEditor::GetInstance()->GetFloatValue("Test1", "testNum");
	testVector = GameParamEditor::GetInstance()->GetVector3Value("Test2", "testVec");
}