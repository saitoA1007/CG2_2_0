#include"PlayerHpUI.h"
#include"EasingManager.h"
#include"FPSCounter.h"
using namespace GameEngine;

void PlayerHpUI::Initialize(const int32_t& maxHp) {

    maxHp_ = maxHp;
    currentHp_ = maxHp_;

    // 個数式UI用にHP数ぶんのスプライトを生成
    hpSprites_.clear();
    hpSprites_.reserve(static_cast<size_t>(maxHp_));

    for (int i = 0; i < maxHp_; ++i) {
        Vector2 pos = { startPos_.x + i * (iconSize_.x + iconSpacing_), startPos_.y };
        auto sprite = Sprite::Create(pos, iconSize_, { 0.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f });
        hpSprites_.push_back(std::move(sprite));
    }
}

void PlayerHpUI::Update() {
    // 現在HPに応じて色を更新
    for (int i = 0; i < maxHp_; ++i) {
        auto& spr = hpSprites_[static_cast<size_t>(i)];
        if (i < currentHp_) {
            spr->SetColor({ 1.0f,1.0f,1.0f,1.0f }); // 残HPは白
        } else {
            spr->SetColor({ 0.0f,0.0f,0.0f,1.0f }); // 減少分は黒
        }
        spr->Update();
    }
}
