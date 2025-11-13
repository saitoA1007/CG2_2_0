#include"GameScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"

#include"GameParamEditor.h"
#include"FPSCounter.h"

using namespace GameEngine;

GameScene::~GameScene() {

}

void GameScene::Initialize(SceneContext* context) {
	// ゲームシーンに必要な低レイヤー機能
#pragma region SceneSystem
	// エンジン機能を取得
	context_ = context;

	// 登録するパラメータを設定
	GameParamEditor::GetInstance()->SetActiveScene("GameScene");

#pragma endregion

	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize(cameraTransform_, 1280, 720, context_->graphicsDevice->GetDevice());

	// 平行光源ライト
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(context_->graphicsDevice->GetDevice(), true, false, false);
	directionalData_.active = true;
	directionalData_.color = { 1.0f,1.0f,1.0f,1.0f };
	directionalData_.direction = { 0.0,0.0f,1.0f };
	directionalData_.intensity = 1.0f;
	lightManager_->SetDirectionalData(directionalData_);

	// 地面モデルを生成
	terrainModel_ = context_->modelManager->GetNameByModel("terrain.obj");
	terrainModel_->SetDefaultIsEnableLight(true);
	grassGH_ = context_->textureManager->GetHandleByName("grass");
	terrainWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });

	// 平面モデルを生成
	planeModel_ = context_->modelManager->GetNameByModel("plane.obj");
	planeModel_->SetDefaultIsEnableLight(true);
	uvCheckerGH_ = context_->textureManager->GetHandleByName("uvChecker");
	planeWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f} });

	// ボーンアニメーションを生成する
	bronAnimationModel_ = context_->modelManager->GetNameByModel("walk.gltf");
	bronAnimationWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
	// ボーンアニメーションデータを取得する
	bronAnimation_ = Model::LoadAnimationFile("walk.gltf", "Walk");
	skeletonBron_ = Model::CreateSkeleton(bronAnimationModel_->modelData_.rootNode);
	skinClusterBron_ = Animation::CreateSkinCluster(skeletonBron_, bronAnimationModel_->modelData_);
	timer_ = 0.0f;

	// 値の保存の登録と適応(テスト)
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

	// カメラの更新処理
	camera_->Update();

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
#endif
}

void GameScene::Draw(const bool& isDebugView) {

	// 描画に使用するカメラを設定
	if (isDebugView) {
		// 描画に使用するカメラを設定
		ModelRenderer::SetCamera(context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
	} else {
		// 描画に使用するカメラを設定
		ModelRenderer::SetCamera(camera_->GetVPMatrix(), camera_->GetCameraResource());
	}

	// 描画に使用するカメラを設定
	ModelRenderer::SetCamera(camera_->GetVPMatrix(), camera_->GetCameraResource());

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// 地面を描画
	ModelRenderer::DrawLight(lightManager_->GetResource());
	ModelRenderer::Draw(terrainModel_, terrainWorldTransform_, grassGH_);

	// 平面描画
	//planeModel_->Draw(planeWorldTransform_, uvCheckerGH_, camera_->GetVPMatrix());

	// アニメーションの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::AnimationModel);

	// アニメーションしているモデルを描画
	ModelRenderer::DrawAnimation(bronAnimationModel_, bronAnimationWorldTransform_, skinClusterBron_);
}

void GameScene::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem("Test1", "testNum", testNumber);
	GameParamEditor::GetInstance()->AddItem("Test2", "testVec", testVector);
}

void GameScene::ApplyDebugParam() {
	testNumber = GameParamEditor::GetInstance()->GetValue<float>("Test1", "testNum");
	testVector = GameParamEditor::GetInstance()->GetValue<Vector3>("Test2", "testVec");
}