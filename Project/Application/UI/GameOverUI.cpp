#include"GameOverUI.h"
#include"AudioManager.h"
using namespace GameEngine;

void GameOverUI::Initialize(GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager) {
	// 入力処理を受け取る
	inputCommand_ = inputCommand;

    // 背景
    bgSprite_ = Sprite::Create({ 0.0f, 0.0f }, { 1280.0f, 720.0f }, { 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f, 0.7f });

    // ロゴスプライト
    logoSprite_ = Sprite::Create({ 640.0f, 180.0f }, { 1024.0f, 256.0f }, { 0.5f, 0.5f });
    logoGH_ = textureManager->GetHandleByName("gameOver.png");

    // やりなおすスプライト
    retrySprite_ = Sprite::Create({ 640.0f, 400.0f }, { 375.0f, 160.0f }, { 0.5f, 0.5f });
    retryGH_ = textureManager->GetHandleByName("retry.png");

    // タイトルにもどるスプライト
    titleSprite_ = Sprite::Create({ 640.0f, 540.0f }, { 375.0f, 160.0f }, { 0.5f, 0.5f });
    titleGH_ = textureManager->GetHandleByName("title.png");

    // 初期選択の要素を設定
    selectNum_ = 0;
    retrySprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
    titleSprite_->color_ = { 0.2f,0.2f,0.2f,1.0f };
    retrySprite_->Update();
    titleSprite_->Update();

    // 選択音
    selectSH_ = AudioManager::GetInstance().GetHandleByName("select.mp3");
    // 決定音を取得
    decisionSH_ = AudioManager::GetInstance().GetHandleByName("decision.mp3");
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
    if (inputCommand_->IsCommandActive("MoveUp")) {
        // リトライ
        selectNum_ = 0;
        // 色を変更
        retrySprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
        titleSprite_->color_ = { 0.2f,0.2f,0.2f,1.0f };

        // 選択音
        if (!AudioManager::GetInstance().IsPlay(selectSH_)) {
            AudioManager::GetInstance().Play(selectSH_, 0.5f, false);
        }
    }

    if (inputCommand_->IsCommandActive("MoveDown")) {
        // タイトル
        selectNum_ = 1;
        // 色を変更
        retrySprite_->color_ = { 0.2f,0.2f,0.2f,1.0f };
        titleSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };

        // 選択音
        if (!AudioManager::GetInstance().IsPlay(selectSH_)) {
            AudioManager::GetInstance().Play(selectSH_, 0.5f, false);
        }
    }

    // 決定
    if (inputCommand_->IsCommandActive("Decision")) {

        // 決定音
        AudioManager::GetInstance().Play(decisionSH_, 0.5f, false);

        if (selectNum_ == 0) {
            // リトライ
            onRetry_();
        } else {
            // タイトル
            onTitle_();
        }
    }
}