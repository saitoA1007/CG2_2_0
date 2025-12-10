#include "BossAppearanceUI.h"
#include "FPSCounter.h"
#include "EngineSource/Math/EasingManager.h"
#include <algorithm>

using namespace GameEngine;

void BossAppearanceUI::Initialize(TextureManager* textureManager) {
    bossTitleTextGH_ = textureManager->GetHandleByName("bossTitleText.png");
    bossTitle1GH_    = textureManager->GetHandleByName("bossTitle1.png");
    bossTitle2GH_    = textureManager->GetHandleByName("bossTitle2.png");
    bossNameTextGH_  = textureManager->GetHandleByName("bossNameText.png");
    bossName1GH_     = textureManager->GetHandleByName("bossName1.png");
    bossName2GH_     = textureManager->GetHandleByName("bossName2.png");
    bossAppearanceStarGH_ = textureManager->GetHandleByName("bossAppearanceStar.png");

    bossTitleSprite_ = Sprite::Create({ 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.5f, 0.5f });
    bossNameSprite_  = Sprite::Create({ 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.5f, 0.5f });
    bossAppearanceStarSprite_ = Sprite::Create(offsetPos_, { 720.0f, 720.0f }, { 0.5f, 0.5f });

    isActive_ = true;
}

void BossAppearanceUI::Update() {
    if (isActive_ && !prevIsActive_) {
        ResetAnimation();
    }
    prevIsActive_ = isActive_;
    UpdateAnimation();

    bossTitleSprite_->Update();
    bossNameSprite_->Update();
    bossAppearanceStarSprite_->Update();
}

void BossAppearanceUI::ResetAnimation() {
    timer_ = 0.0f;
    bossTitleSprite_->SetPosition(offsetPos_ + Vector2{ 320.0f, -720.0f });
    bossNameSprite_->SetPosition(offsetPos_ + Vector2{ -320.0f, 720.0f });
    bossTitleSprite_->SetSize({ 240.0f, 240.0f });
    bossNameSprite_->SetSize({ 240.0f, 240.0f });
    bossTitleSprite_->color_.w = 1.0f;
    bossNameSprite_->color_.w = 1.0f;
    bossAppearanceStarSprite_->color_.w = 0.0f;
    currentBossTitleGH_ = bossTitle1GH_;
    currentBossNameGH_ = bossName1GH_;
}

void BossAppearanceUI::UpdateAnimation() {
    if (!isActive_) { return; }
    timer_ += FpsCounter::deltaTime;
    if (timer_ > animationDuration_) { timer_ = animationDuration_; }

    // 0.0f ～ 0.3f
    {
        float startT = 0.0f;
        float endT = 0.2f;
        if (timer_ >= startT && timer_ <= endT) {
            float t = std::clamp((timer_ - startT) / (endT - startT), 0.0f, 1.0f);
            Vector2 nameFrom = offsetPos_ + Vector2{ -320.0f, 720.0f };
            Vector2 nameTo   = offsetPos_ + Vector2{ -320.0f, -180.0f };
            Vector2 titleFrom = offsetPos_ + Vector2{ 320.0f, -720.0f };
            Vector2 titleTo   = offsetPos_ + Vector2{ 320.0f, 180.0f };
            bossNameSprite_->SetPosition(EaseOutExpo(nameFrom, nameTo, t));
            bossTitleSprite_->SetPosition(EaseOutExpo(titleFrom, titleTo, t));
        }
    }

    // 0.3f ～ 0.6f
    {
        float startT = 0.2f;
        float endT = 0.4f;
        if (timer_ >= startT && timer_ <= endT) {
            float t = std::clamp((timer_ - startT) / (endT - startT), 0.0f, 1.0f);
            Vector2 nameFrom = offsetPos_ + Vector2{ -320.0f, -180.0f };
            Vector2 nameTo   = offsetPos_ + Vector2{ 320.0f, -180.0f };
            Vector2 titleFrom = offsetPos_ + Vector2{ 320.0f, 180.0f };
            Vector2 titleTo   = offsetPos_ + Vector2{ -320.0f, 180.0f };
            bossNameSprite_->SetPosition(EaseInOutExpo(nameFrom, nameTo, t));
            bossTitleSprite_->SetPosition(EaseInOutExpo(titleFrom, titleTo, t));
            if (t >= 0.5f) {
                currentBossTitleGH_ = bossTitle2GH_;
                currentBossNameGH_ = bossName2GH_;
            }
        }
    }

    // 0.6f ～ 0.9f (位置 + アルファ 1→0)
    {
        float startT = 0.4f;
        float endT = 0.6f;
        if (timer_ >= startT && timer_ <= endT) {
            float t = std::clamp((timer_ - startT) / (endT - startT), 0.0f, 1.0f);
            Vector2 nameFrom = offsetPos_ + Vector2{ 320.0f, -180.0f };
            Vector2 nameTo   = offsetPos_ + Vector2{ 320.0f, 180.0f };
            Vector2 titleFrom = offsetPos_ + Vector2{ -320.0f, 180.0f };
            Vector2 titleTo   = offsetPos_ + Vector2{ -320.0f, -180.0f };
            bossNameSprite_->SetPosition(EaseInOutExpo(nameFrom, nameTo, t));
            bossTitleSprite_->SetPosition(EaseInOutExpo(titleFrom, titleTo, t));

            float easedAlpha = EaseInOutExpo(1.0f, 0.0f, t);
            bossNameSprite_->color_.w = easedAlpha;
            bossTitleSprite_->color_.w = easedAlpha;
        }
    }

    // 0.7f ～ 1.0f (星: 表示しつつ時計回り360度 EaseOutExpo)
    {
        float startT = 0.5f;
        float endT = 0.8f;
        if (timer_ >= startT && timer_ <= endT) {
            float t = std::clamp((timer_ - startT) / (endT - startT), 0.0f, 1.0f);
            bossAppearanceStarSprite_->color_.w = 1.0f;
            const float twoPi = 6.28318530717958647692f;
            bossAppearanceStarSprite_->rotate_ = EaseOutExpo(0.0f, twoPi, t);
        }
    }

    // 1.0f ～ 1.3f (星: 表示しつつ時計回り360度 EaseInExpo, 終了後に非表示)
    {
        float startT = 0.8f;
        float endT = 1.1f;
        const float twoPi = 6.28318530717958647692f;
        if (timer_ >= startT && timer_ <= endT) {
            float t = std::clamp((timer_ - startT) / (endT - startT), 0.0f, 1.0f);
            bossAppearanceStarSprite_->color_.w = 1.0f;
            bossAppearanceStarSprite_->rotate_ = twoPi + EaseInExpo(0.0f, twoPi, t);
        }
        if (timer_ > endT) {
            bossAppearanceStarSprite_->color_.w = 0.0f;
        }
    }

    // 1.2f ～ 1.5f (タイトル・ネーム: サイズ固定、原点から各方向に EaseOutExpo)
    {
        float startT = 1.0f;
        float endT = 1.3f;
        if (timer_ >= startT && timer_ <= endT) {
            float t = std::clamp((timer_ - startT) / (endT - startT), 0.0f, 1.0f);
            bossNameSprite_->SetSize({ 1280.0f, 200.0f });
            bossTitleSprite_->SetSize({ 1280.0f, 200.0f });
            bossNameSprite_->color_.w = 1.0f;
            bossTitleSprite_->color_.w = 1.0f;
            currentBossNameGH_ = bossNameTextGH_;
            currentBossTitleGH_ = bossTitleTextGH_;

            Vector2 nameFrom = offsetPos_ + Vector2{ 0.0f, 0.0f };
            Vector2 nameTo   = offsetPos_ + Vector2{ 0.0f, 300.0f };
            Vector2 titleFrom = offsetPos_ + Vector2{ 0.0f, 0.0f };
            Vector2 titleTo   = offsetPos_ + Vector2{ 0.0f, -300.0f };
            bossNameSprite_->SetPosition(EaseOutExpo(nameFrom, nameTo, t));
            bossTitleSprite_->SetPosition(EaseOutExpo(titleFrom, titleTo, t));
        }
    }

    // 2.5f ～ 3.0f (タイトル・ネーム: 画面外へ EaseInExpo)
    {
        float startT = 2.5f;
        float endT = animationDuration_;
        if (timer_ >= startT && timer_ <= endT) {
            float t = std::clamp((timer_ - startT) / (endT - startT), 0.0f, 1.0f);
            Vector2 nameFrom = offsetPos_ + Vector2{ 0.0f, 300.0f };
            Vector2 nameTo   = offsetPos_ + Vector2{ 0.0f, 720.0f };
            Vector2 titleFrom = offsetPos_ + Vector2{ 0.0f, -300.0f };
            Vector2 titleTo   = offsetPos_ + Vector2{ 0.0f, -720.0f };
            bossNameSprite_->SetPosition(EaseInExpo(nameFrom, nameTo, t));
            bossTitleSprite_->SetPosition(EaseInExpo(titleFrom, titleTo, t));
        }
    }
}