#include"GameOverUI.h"

using namespace GameEngine;

void GameOverUI::Initialize(GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager) {
	// 入力処理を受け取る
	inputCommand_ = inputCommand;

    // 背景
    bgSprite_ = Sprite::Create({ 0.0f, 0.0f }, { 1280.0f, 720.0f }, { 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f, 0.7f });

    // ロゴスプライト
    logoSprite_ = Sprite::Create({ 640.0f, 180.0f }, { 1024.0f, 256.0f }, { 0.5f, 0.5f });
    logoGH_ = textureManager->GetHandleByName("gameOverText.png");

    // やりなおすスプライト
    retrySprite_ = Sprite::Create({ 640.0f, 360.0f }, { 375.0f, 80.0f }, { 0.5f, 0.5f });
    retryGH_ = textureManager->GetHandleByName("retryText.png");

    // タイトルにもどるスプライト
    titleSprite_ = Sprite::Create({ 640.0f, 488.0f }, { 500.0f, 80.0f }, { 0.5f, 0.5f });
    titleGH_ = textureManager->GetHandleByName("titleBackText.png");

    // 初期選択の要素を設定
    selectNum_ = 0;
    retrySprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
    titleSprite_->color_ = { 0.2f,0.2f,0.2f,1.0f };
    retrySprite_->Update();
    titleSprite_->Update();
}

void GameOverUI::Update() {

    // 入力処理
    ProcessInput();

    // スプライトの更新処理
    bgSprite_->Update();
    logoSprite_->Update();
    retrySprite_->Update();
    titleSprite_->Update();
}

void GameOverUI::ProcessInput() {

    // 選択
    if (inputCommand_->IsCommandAcitve("MoveUp")) {
        // リトライ
        selectNum_ = 0;
        // 色を変更
        retrySprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
        titleSprite_->color_ = { 0.2f,0.2f,0.2f,1.0f };
    }

    if (inputCommand_->IsCommandAcitve("MoveDown")) {
        // タイトル
        selectNum_ = 1;
        // 色を変更
        retrySprite_->color_ = { 0.2f,0.2f,0.2f,1.0f };
        titleSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
    }

    // 決定
    if (inputCommand_->IsCommandAcitve("Decision")) {
        if (selectNum_ == 0) {
            // リトライ
            onRetry_();
        } else {
            // タイトル
            onTitle_();
        }
    }
}