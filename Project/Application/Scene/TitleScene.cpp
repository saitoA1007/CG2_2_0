#include"TitleScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"GameParamEditor.h"
#include"SpriteRenderer.h"
#include"Extension/CustomRenderer.h"
#include"AudioManager.h"
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

#pragma endregion

	// メインカメラの初期化
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720, context_->graphicsDevice->GetDevice());

	// 天球モデルを生成
	skyDomeModel_ = context_->modelManager->GetNameByModel("SkyDome");
	skyDomeWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// ライトの生成
	sceneLightingController_ = std::make_unique<SceneLightingController>();
	sceneLightingController_->Initialize(context_->graphicsDevice->GetDevice());

	// ボスモデルを生成
	bossEnemyModel_ = context_->modelManager->GetNameByModel("BossBody");
	bossEnemyEyeModel_ = context_->modelManager->GetNameByModel("BossEye");
	bossEnemyEyeModel_->SetDefaultColor({ 0.0f,0.0f,0.0f,1.0f });
	// 行列を初期化
	bossWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
	// マテリアルの初期化
	iceMaterial_ = std::make_unique<IceRockMaterial>();
	iceMaterial_->Initialize();
	iceMaterial_->materialData_->textureHandle = context_->textureManager->GetHandleByName("boss.png");

	// タイトル画像
	titleSprite_ = Sprite::Create({640.0f,250.0f},{600.0f,128.0f},{0.5f,0.5f});
	titleGH_ = context_->textureManager->GetHandleByName("titleText.png");

	// スペースボタン
	spaceSprite_ = Sprite::Create({ 640.0f,500.0f }, { 256.0f * 1.2f,64.0f*1.2f }, { 0.5f,0.5f });
	spaceGH_ = context_->textureManager->GetHandleByName("press.png");

	// 音声を取得
	titleSH_ = AudioManager::GetInstance().GetHandleByName("titleBGM.mp3");

	// 決定音を取得
	decisionSH_ = AudioManager::GetInstance().GetHandleByName("decision.mp3");
}

void TitleScene::Update() {

	// BGMを再生
	if (!AudioManager::GetInstance().IsPlay(titleSH_)) {
		AudioManager::GetInstance().Play(titleSH_, 0.3f, false);
	}

	if (context_->input->TriggerKey(DIK_SPACE) || context_->input->TriggerPad(XINPUT_GAMEPAD_A)) {
		isFinished_ = true;

		// 決定音
		AudioManager::GetInstance().Play(decisionSH_, 0.5f, false);

		// BGMを停止
		if (AudioManager::GetInstance().IsPlay(titleSH_)) {
			AudioManager::GetInstance().Stop(titleSH_);
		}
	}
	
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

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// 天球の描画
	ModelRenderer::Draw(skyDomeModel_, skyDomeWorldTransform_);

	// ボスの体を描画
	CustomRenderer::PreDraw(CustomRenderMode::RockBoth);
	CustomRenderer::DrawRock(bossEnemyModel_, bossWorldTransform_, sceneLightingController_->GetResource(), iceMaterial_.get());
	// ボスの目を描画
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);
	ModelRenderer::Draw(bossEnemyEyeModel_, bossWorldTransform_);

	//======================================================
	// 2D描画
	//======================================================

	// 画像の描画前処理
	SpriteRenderer::PreDraw(RenderMode2D::Normal);

	// タイトル描画
	SpriteRenderer::Draw(titleSprite_.get(), titleGH_);

	// スペース画像
	SpriteRenderer::Draw(spaceSprite_.get(), spaceGH_);
}
