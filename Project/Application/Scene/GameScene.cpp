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

	// デフォルトで描画するパス
	context->renderPassController->AddPass("DefaultPass", true);
	// 最終的な描画先を設定
	context_->renderPassController->SetEndPass("DefaultPass");
#pragma endregion

	// メインカメラの初期化
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720, context_->graphicsDevice->GetDevice());

	// 平行光源ライト
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(context_->graphicsDevice->GetDevice(), true, false, false);
	directionalData_.active = true;
	directionalData_.color = { 1.0f,1.0f,1.0f,1.0f };
	directionalData_.direction = { 0.0,0.0f,1.0f };
	directionalData_.intensity = 1.0f;
	lightManager_->SetDirectionalData(directionalData_);

	// 地面モデルを生成
	terrainModel_ = context_->modelManager->GetNameByModel("Terrain");
	terrainModel_->SetDefaultIsEnableLight(true);
	grassGH_ = context_->textureManager->GetHandleByName("grass.png");
	terrainWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });

	// 平面モデルを生成
	planeModel_ = context_->modelManager->GetNameByModel("Plane");
	planeModel_->SetDefaultIsEnableLight(true);
	uvCheckerGH_ = context_->textureManager->GetHandleByName("uvChecker.png");
	planeWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f} });

	// ボーンアニメーションを生成する
	bronAnimationModel_ = context_->modelManager->GetNameByModel("Walk");
	bronAnimationWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// 歩くアニメーションデータを取得する
	walkAnimationData_ = context_->animationManager->GetNameByAnimations("Walk");
	// 歩くアニメーションの再生を管理する
	walkAnimator_ = std::make_unique<Animator>();
	walkAnimator_->Initialize(bronAnimationModel_, &walkAnimationData_["Armature|mixamo.com|Layer0"]);
	
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

	// 歩くアニメーションの更新処理
	walkAnimator_->Update();

	// カメラの更新処理
	mainCamera_->Update();

#ifdef _DEBUG

	// 光源をデバック
	ImGui::Begin("DebugWindow");
	// カメラのデバック
	ImGui::DragFloat3("CameraTranslate", &mainCamera_->transform_.translate.x, 0.01f);
	ImGui::DragFloat3("CameraRotate", &mainCamera_->transform_.translate.x, 0.01f);

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
		ModelRenderer::SetCamera(mainCamera_->GetVPMatrix(), mainCamera_->GetCameraResource());
	}

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// 地面を描画
	ModelRenderer::DrawLight(lightManager_->GetResource());
	terrainModel_->SetDefaultTextureHandle(grassGH_);
	ModelRenderer::Draw(terrainModel_, terrainWorldTransform_);

	// アニメーションの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::AnimationModel);

	// アニメーションしているモデルを描画
	ModelRenderer::DrawAnimation(bronAnimationModel_, bronAnimationWorldTransform_);
}

void GameScene::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem("Test1", "testNum", testNumber);
	GameParamEditor::GetInstance()->AddItem("Test2", "testVec", testVector);
}

void GameScene::ApplyDebugParam() {
	testNumber = GameParamEditor::GetInstance()->GetValue<float>("Test1", "testNum");
	testVector = GameParamEditor::GetInstance()->GetValue<Vector3>("Test2", "testVec");
}