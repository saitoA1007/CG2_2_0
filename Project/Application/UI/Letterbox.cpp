#include "Letterbox.h"
#include "EngineSource/Math/Vector2.h"

using namespace GameEngine;

void Letterbox::Initialize(float screenWidth, float screenHeight, float boxHeight, const Vector4& color) {
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    boxHeight_ = boxHeight;
    color_ = color;

    topSprite_ = Sprite::Create(
        Vector2{ screenWidth_ * 0.5f, 0.0f },
        Vector2{ screenWidth_, 0.0f },
        Vector2{ 0.5f, 0.0f },
        color_);

    bottomSprite_ = Sprite::Create(
        Vector2{ screenWidth_ * 0.5f, screenHeight_ },
        Vector2{ screenWidth_, 0.0f },
        Vector2{ 0.5f, 1.0f },
        color_);

    SetBoxHeight(boxHeight_);
}

void Letterbox::SetBoxHeight(float boxHeight) {
    boxHeight_ = boxHeight;
    if (topSprite_) {
        topSprite_->SetSize(Vector2{ screenWidth_, boxHeight_ });
    }
    if (bottomSprite_) {
        bottomSprite_->SetSize(Vector2{ screenWidth_, boxHeight_ });
    }
}

void Letterbox::Update() {
    if (topSprite_) topSprite_->Update();
    if (bottomSprite_) bottomSprite_->Update();
}
