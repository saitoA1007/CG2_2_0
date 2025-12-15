#define NOMINMAX
#include "PlayerDamageNumberEffect.h"
#include "MyMath.h"
#include "FPSCounter.h"
#include "EasingManager.h"
#include <algorithm>

using namespace GameEngine;

void PlayerDamageNumberEffect::Initialize(TextureManager* textureManager) {
    textureManager_ = textureManager;
    isActive_ = false;
    timer_ = 0.0f;
}

static uint32_t GetDigitTextureHandle(GameEngine::TextureManager* tm, int digit) {
    if (!tm) return 0;
    digit = std::clamp(digit, 0, 9);
    // ClearUIと同じ数字テクスチャ（"0.png" ～ "9.png"）
    std::string name = std::to_string(digit) + ".png";
    return tm->GetHandleByName(name);
}

void PlayerDamageNumberEffect::BuildDigits(int damage) {
    digits_.clear();
    digitSprites_.clear();
    digitTextureHandles_.clear();

    if (damage <= 0) {
        damage = std::max(damage, 0);
    }
    std::string s = std::to_string(damage);

    float digitWidth = digitSize_.x;
    float digitHeight = digitSize_.y;
    float startX = -(digitWidth * 0.5f) * static_cast<float>((s.size() > 0) ? (s.size() - 1) : 0);

    digits_.reserve(s.size());
    digitSprites_.reserve(s.size());
    digitTextureHandles_.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i) {
        int d = s[i] - '0';
        uint32_t h = GetDigitTextureHandle(textureManager_, d);
        DigitSprite ds;
        ds.number = h;
        ds.offset = { startX + digitWidth * static_cast<float>(i), 0.0f };
        ds.numSprite_ = Sprite::Create({ 0.0f, 0.0f }, { digitWidth, digitHeight }, { 0.5f, 0.5f }, digitColor_);
        digitSprites_.push_back(ds.numSprite_.get());
        digitTextureHandles_.push_back(h);
        digits_.push_back(std::move(ds));
    }
}

Vector2 PlayerDamageNumberEffect::WorldToScreen(const Vector3& worldPos, const Camera& camera) {
    Matrix4x4 vp = camera.GetVPMatrix();
    const float viewportWidth = 1280.0f;
    const float viewportHeight = 720.0f;
    Vector3 screen = Project(worldPos, {0.0f, 0.0f}, viewportWidth, viewportHeight, vp);
    return { screen.x, screen.y };
}

void PlayerDamageNumberEffect::Emitter(const Vector3& worldPos, int damage, const Camera& camera) {
    spawnScreenPos_ = WorldToScreen(worldPos, camera);
    BuildDigits(damage);
    timer_ = 0.0f;
    isActive_ = true;
}

void PlayerDamageNumberEffect::Update() {
    if (!isActive_) return;
    timer_ += GameEngine::FpsCounter::deltaTime / std::max(duration_, 0.0001f);
    if (timer_ >= 1.0f) {
        timer_ = 1.0f;
        isActive_ = false;
    }

    float yPos = spawnScreenPos_.y;
    float alpha = 1.0f;
    if (timer_ <= 0.5f) {
        float t = std::clamp(timer_ / 0.5f, 0.0f, 1.0f);
        yPos = EaseOutCubic(spawnScreenPos_.y, spawnScreenPos_.y - 64.0f, t);
        alpha = 1.0f;
    } else {
        float t = std::clamp((timer_ - 0.5f) / 0.5f, 0.0f, 1.0f);
        yPos = EaseInCubic(spawnScreenPos_.y - 64.0f, spawnScreenPos_.y + 32.0f, t);
        alpha = EaseInCubic(1.0f, 0.0f, t);
    }

    for (auto& d : digits_) {
        if (d.numSprite_) {
            d.numSprite_->position_ = { spawnScreenPos_.x + d.offset.x, yPos + d.offset.y };
            d.numSprite_->size_ = digitSize_;
            d.numSprite_->color_ = digitColor_;
            d.numSprite_->color_.w = alpha;
            d.numSprite_->Update();
        }
    }
}