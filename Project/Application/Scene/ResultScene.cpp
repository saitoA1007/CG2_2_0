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

	InputRegisterCommand();

	// メインカメラの初期化
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720, context_->graphicsDevice->GetDevice());

	// タイトル画像
	titleSprite_ = Sprite::Create({ 640.0f,250.0f }, { 600.0f,128.0f }, { 0.5f,0.5f });
	titleGH_ = context_->textureManager->GetHandleByName("clearText.png");

	// スペースボタン
	spaceSprite_ = Sprite::Create({ 640.0f,500.0f }, { 256.0f,64.0f }, { 0.5f,0.5f });
	spaceGH_ = context_->textureManager->GetHandleByName("spaceText.png");
}

void ResultScene::Update() {

	// 仮の遷移
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
	//ModelRenderer::PreDraw(RenderMode::DefaultModel);

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

void ResultScene::InputRegisterCommand() {
	// Startゲーム開始（EnterキーまたはAボタン）
	context_->inputCommand->RegisterCommand("Start", { {InputState::KeyTrigger, DIK_RETURN}, {InputState::PadTrigger, XINPUT_GAMEPAD_A} });
	// メニュー移動
	context_->inputCommand->RegisterCommand("Up", { {InputState::KeyTrigger, DIK_UP}, {InputState::PadTrigger, XINPUT_GAMEPAD_DPAD_UP} });
	context_->inputCommand->RegisterCommand("Down", { {InputState::KeyTrigger, DIK_DOWN}, {InputState::PadTrigger, XINPUT_GAMEPAD_DPAD_DOWN} });
	// バック/終了
	context_->inputCommand->RegisterCommand("Back", { {InputState::KeyTrigger, DIK_ESCAPE}, {InputState::PadTrigger, XINPUT_GAMEPAD_B} });
}

