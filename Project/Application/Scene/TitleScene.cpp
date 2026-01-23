#include"TitleScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"GameParamEditor.h"
using namespace GameEngine;

TitleScene::~TitleScene() {
}

void TitleScene::Initialize(SceneContext* context) {
	// ゲームシーンに必要な低レイヤー機能
#pragma region SceneSystem 
	// エンジン機能を取得
	context_ = context;

	// 登録するパラメータを設定
	GameParamEditor::GetInstance()->SetActiveScene("TitleScene");

	// デフォルトで描画するパス
	context_->renderPassController->AddPass("DefaultPass",true);
	// 最終的な描画先を設定
	context_->renderPassController->SetEndPass("DefaultPass");
#pragma endregion

	// メインカメラの初期化
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720, context_->graphicsDevice->GetDevice());

	// ライトの初期化
	sceneLightingController_ = std::make_unique<SceneLightingController>();
	sceneLightingController_->Initialize(context_->graphicsDevice->GetDevice());

	// スカイボックスの生成
	skyboxModel_ = context_->modelManager->GetNameByModel("Skybox");
	skyboxWorldTransform_.Initialize({ {100.0f,100.0f,100.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
	skyboxGH_ = context_->textureManager->GetHandleByName("rostock_laage_airport_4k.dds");
	skyboxModel_->SetDefaultTextureHandle(skyboxGH_);
	// 環境マップを設定
	sceneLightingController_->SetEnvironment(skyboxGH_);

	// 地面モデルを生成
	terrainModel_ = context_->modelManager->GetNameByModel("Terrain");
	terrainModel_->SetDefaultIsEnableLight(true);
	grassGH_ = context_->textureManager->GetHandleByName("grass.png");
	terrainModel_->SetDefaultTextureHandle(grassGH_);
	terrainModel_->SetDefaultMetallic(terrainMetalic_);
	terrainModel_->SetDefaultShiness(terrainShininess_);
	terrainWorldTransform_.Initialize({ {2.0f,2.0f,2.0f},{0.0f,-1.6f,0.0f},{0.0f,-2.0f,0.0f} });

	// 画像を取得
	monsterGH_ = context_->textureManager->GetHandleByName("monsterBall.png");
	// 球の初期化
	sphereModel_ = context_->modelManager->GetNameByModel("Sphere");
	sphereModel_->SetDefaultIsEnableLight(true);
	sphereModel_->SetDefaultTextureHandle(monsterGH_);
	sphereModel_->SetDefaultMetallic(sphereMetalic_);
	sphereModel_->SetDefaultShiness(sphereShininess_);
	sphereWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.0f,0.0f,0.0f} });

	// objのplaneモデル
	objPlaneModel_ = context_->modelManager->GetNameByModel("ObjPlane");
	objPlaneModel_->SetDefaultIsEnableLight(true);
	objPlaneWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{-3.0f,0.0f,0.0f} });

	// gltfのplaneモデル
	gltfPlaneModel_ = context_->modelManager->GetNameByModel("GltfPlane");
	gltfPlaneModel_->SetDefaultIsEnableLight(true);
	gltfPlaneWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{3.0f,0.0f,0.0f} });

	RegisterDebugParam();
}

void TitleScene::Update() {
	ApplyDebugParam();

	// ライトの更新処理
	sceneLightingController_->Update();

	// カメラの更新処理
	mainCamera_->Update();
}

void TitleScene::Draw(const bool& isDebugView) {

	// 描画に使用するカメラを設定
	if (isDebugView) {
		// 描画に使用するカメラを設定
		ModelRenderer::SetCamera(context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
	} else {
		// 描画に使用するカメラを設定
		ModelRenderer::SetCamera(mainCamera_->GetVPMatrix(), mainCamera_->GetCameraResource());
	}

	// 描画パスの管理を取得
	auto pass = context_->renderPassController;

	// 描画パス
	pass->PrePass("DefaultPass");

	//===========================================================
	// 3D描画
	//===========================================================

	// スカイボックスの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::Skybox);

	ModelRenderer::DrawSkybox(skyboxModel_, skyboxWorldTransform_);

	// 描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// 地面を描画
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(terrainModel_, terrainWorldTransform_);

	// 球の描画
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(sphereModel_, sphereWorldTransform_);

	// objのplaneモデルを描画
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(objPlaneModel_, objPlaneWorldTransform_);

	// gltfのplaneモデルを描画
	ModelRenderer::DrawLight(sceneLightingController_->GetResource());
	ModelRenderer::Draw(gltfPlaneModel_, gltfPlaneWorldTransform_);

	pass->PostPass("DefaultPass");
}

void TitleScene::DebugUpdate() {
	ApplyDebugParam();

	// ライトの更新処理
	sceneLightingController_->Update();
}

void TitleScene::RegisterDebugParam() {
	GameParamEditor::GetInstance()->AddItem("Sphere", "Metalic", sphereMetalic_);
	GameParamEditor::GetInstance()->AddItem("Sphere", "Shininess", sphereShininess_);

	GameParamEditor::GetInstance()->AddItem("Terrain", "Metalic", terrainMetalic_);
	GameParamEditor::GetInstance()->AddItem("Terrain", "Shininess", terrainShininess_);
}

void TitleScene::ApplyDebugParam() {
	sphereMetalic_ = GameParamEditor::GetInstance()->GetValue<float>("Sphere", "Metalic");
	sphereShininess_ = GameParamEditor::GetInstance()->GetValue<float>("Sphere", "Shininess");
	sphereModel_->SetDefaultMetallic(sphereMetalic_);
	sphereModel_->SetDefaultShiness(sphereShininess_);

	terrainMetalic_ = GameParamEditor::GetInstance()->GetValue<float>("Terrain", "Metalic");
	terrainShininess_ = GameParamEditor::GetInstance()->GetValue<float>("Terrain", "Shininess");
	terrainModel_->SetDefaultMetallic(terrainMetalic_);
	terrainModel_->SetDefaultShiness(terrainShininess_);
}
