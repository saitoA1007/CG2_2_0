#include"ResultScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"GameParamEditor.h"
#include"SpriteRenderer.h"
using namespace GameEngine;

ResultScene::~ResultScene() {
}

void ResultScene::Initialize(SceneContext* context) {
	// ゲームシーンに必要な低レイヤー機能
#pragma region SceneSystem 
	// エンジン機能を取得
	context_ = context;

	// 登録するパラメータを設定
	GameParamEditor::GetInstance()->SetActiveScene("ResultScene");

#pragma endregion

	// メインカメラの初期化
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720, context_->graphicsDevice->GetDevice());

	// 天球モデルを生成
	skyDomeModel_ = context_->modelManager->GetNameByModel("SkyDome");
	skyDomeWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// タイトル画像
	titleSprite_ = Sprite::Create({ 640.0f,200.0f }, { 600.0f,128.0f }, { 0.5f,0.5f },{0.8f,0.8f,0.0f,1.0f});
	titleGH_ = context_->textureManager->GetHandleByName("clearText.png");

	// スペースボタン
	spaceSprite_ = Sprite::Create({ 640.0f,540.0f }, { 256.0f,64.0f }, { 0.5f,0.5f });
	spaceGH_ = context_->textureManager->GetHandleByName("press.png");

	// タイム
	timeSprite_ = Sprite::Create({ 640.0f,340.0f }, { 256.0f * 0.8f,128.0f*0.8f }, { 0.5f,0.5f });
	timeGH_ = context_->textureManager->GetHandleByName("time.png");

	// クリア時間UI
	clearTimeUI_ = std::make_unique<ClearTimeUI>(context_->textureManager);
}

void ResultScene::Update() {

	if (context_->input->TriggerKey(DIK_SPACE) || context_->input->TriggerPad(XINPUT_GAMEPAD_A)) {
		isFinished_ = true;
	}

	// カメラの更新処理
	mainCamera_->Update();
}

void ResultScene::Draw(const bool& isDebugView) {

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

	//======================================================
	// 2D描画
	//======================================================

	// 画像の描画前処理
	SpriteRenderer::PreDraw(RenderMode2D::Normal);

	// タイトル描画
	SpriteRenderer::Draw(titleSprite_.get(), titleGH_);

	// スペース画像
	SpriteRenderer::Draw(spaceSprite_.get(), spaceGH_);

	// タイム
	SpriteRenderer::Draw(timeSprite_.get(), timeGH_);

	// クリアUI
	SpriteRenderer::Draw(clearTimeUI_->GetnumDottoSprite(), clearTimeUI_->GetDottoTexture());

	// 時間を表示
	for (auto& num : clearTimeUI_->GetNumberSprite()) {
		SpriteRenderer::Draw(num.numSprite_.get(), num.number);
	}
}
