#define NOMINMAX
#include "GameOverUI.h"
#include "SpriteRenderer.h"
#include "GameParamEditor.h"
#include "EngineSource/Core/FPSCounter.h"
#include "EasingManager.h"
#include <algorithm>

using namespace GameEngine;

void GameOverUI::Initialize(Input* input, InputCommand* inputCommand, TextureManager* textureManager) {
    input_ = input;
    inputCommand_ = inputCommand;

    // 背景スプライト（画面全体）
    bgSprite_ = Sprite::Create({ 640.0f, 360.0f }, { 1280.0f, 720.0f }, { 0.5f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 0.75f });

    // ロゴスプライト
    logoSprite_ = Sprite::Create({ 640.0f, 180.0f }, { 1024.0f, 256.0f }, { 0.5f, 0.5f });
    logoGH_ = textureManager->GetHandleByName("gameOverText.png");

    // やりなおすスプライト
    retrySprite_ = Sprite::Create(retryPos_, retrySize_, { 0.5f, 0.5f });
    retryGH_ = textureManager->GetHandleByName("retryText.png");

    // タイトルにもどるスプライト
    titleSprite_ = Sprite::Create(titlePos_, titleSize_, { 0.5f, 0.5f });
    titleGH_ = textureManager->GetHandleByName("titleBackText.png");
}

void GameOverUI::Update() {
    if (!isActive_) return;

    // 有効化演出の更新
    UpdateActivateAnimation();

    // スプライトの更新
    if (bgSprite_) bgSprite_->Update();
    if (logoSprite_) logoSprite_->Update();
    if (retrySprite_) retrySprite_->Update();
    if (titleSprite_) titleSprite_->Update();

    // 入力処理（ホバー/クリック判定）
    HandleInput();
}

void GameOverUI::HandleInput() {
    if (!input_) return;

    Vector2 mousePos = input_->GetMousePosition();

    auto isInRect = [&](const Vector2& center, const Vector2& size, const Vector2& p) {
        Vector2 half = { size.x * 0.5f, size.y * 0.5f };
        Vector2 min = { center.x - half.x, center.y - half.y };
        Vector2 max = { center.x + half.x, center.y + half.y };
        return (p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y);
    };

    // ホバー判定
    isHoverRetry_ = isInRect(retryPos_, retrySize_, mousePos);
    isHoverTitle_ = isInRect(titlePos_, titleSize_, mousePos);

    // マウス左クリックトリガー
    bool click = input_->TriggerMouse(0);

    if (click) {
        if (isHoverRetry_ && onRetryClicked_) {
            onRetryClicked_();
        }
        if (isHoverTitle_ && onTitleClicked_) {
            onTitleClicked_();
        }
    }
}

void GameOverUI::SetActive(bool active) {
    if (active && !isActive_) {
        isActive_ = true;
        StartActivateAnimation();
    } else if (!active && isActive_) {
        isActive_ = false;
        isActivating_ = false;
        activateAnimTimer_ = 0.0f;
        // 非表示時位置をリセット
        if (bgSprite_) bgSprite_->SetPosition({ 640.0f, 360.0f });
        if (logoSprite_) logoSprite_->SetPosition({ 640.0f, 180.0f });
        if (retrySprite_) retrySprite_->SetPosition(retryPos_);
        if (titleSprite_) titleSprite_->SetPosition(titlePos_);
    } else {
        isActive_ = active;
    }
}

void GameOverUI::StartActivateAnimation() {
    isActivating_ = true;
    activateAnimTimer_ = 0.0f;
    // 初期位置設定
    if (bgSprite_) bgSprite_->SetPosition({ 640.0f, 0.0f });
    if (logoSprite_) logoSprite_->SetPosition({ 640.0f, -128.0f });
    if (retrySprite_) retrySprite_->SetPosition({ -640.0f, retryPos_.y });
    if (titleSprite_) titleSprite_->SetPosition({ -640.0f, titlePos_.y });
}

static inline float Remap01(float v, float start, float end) {
    if (end <= start) return 1.0f;
    float t = (v - start) / (end - start);
    return std::clamp(t, 0.0f, 1.0f);
}

void GameOverUI::UpdateActivateAnimation() {
    if (!isActivating_) return;

    // 時間更新
    activateAnimTimer_ += FpsCounter::deltaTime;
    float tAll = std::clamp(activateAnimTimer_ / std::max(activateAnimDuration_, 0.0001f), 0.0f, 1.0f);

    // 背景: 0.0f ～ 0.8f に EaseOutBounce で Y を 0 -> 720
    {
        float lt = Remap01(tAll, 0.0f, 0.8f);
        float eased = EaseOutBounce(0.0f, 1.0f, lt);
        float y = Lerp(0.0f, 720.0f, eased);
        if (bgSprite_) bgSprite_->SetPosition({ 640.0f, y });
    }

    // ロゴ: 0.2f ～ 1.0f に EaseOutBounce で Y を -128 -> 180
    {
        float lt = Remap01(tAll, 0.2f, 1.0f);
        float eased = EaseOutBounce(0.0f, 1.0f, lt);
        float y = Lerp(-128.0f, 180.0f, eased);
        if (logoSprite_) logoSprite_->SetPosition({ 640.0f, y });
    }

    // リトライ: 0.4f ～ 0.8f に EaseOutBack で X を -640 -> 640
    {
        float lt = Remap01(tAll, 0.4f, 0.8f);
        float eased = EaseOutBack(0.0f, 1.0f, lt);
        float x = Lerp(-640.0f, 640.0f, eased);
        if (retrySprite_) retrySprite_->SetPosition({ x, retryPos_.y });
    }

    // タイトル: 0.6f ～ 1.0f に EaseOutBack で X を -640 -> 640
    {
        float lt = Remap01(tAll, 0.6f, 1.0f);
        float eased = EaseOutBack(0.0f, 1.0f, lt);
        float x = Lerp(-640.0f, 640.0f, eased);
        if (titleSprite_) titleSprite_->SetPosition({ x, titlePos_.y });
    }

    if (tAll >= 1.0f) {
        isActivating_ = false;
        // 最終位置補正
        if (bgSprite_) bgSprite_->SetPosition({ 640.0f, 720.0f });
        if (logoSprite_) logoSprite_->SetPosition({ 640.0f, 180.0f });
        if (retrySprite_) retrySprite_->SetPosition(retryPos_);
        if (titleSprite_) titleSprite_->SetPosition(titlePos_);
    }
}