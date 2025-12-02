#include"BossHpUI.h"

using namespace GameEngine;

void BossHpUI::Initialize(const uint32_t& maxHp) {
	maxHp_ = maxHp;
	currentHp_ = maxHp_;

	// 初期化
	sprite_ = Sprite::Create({320.0f,32.0f},{640.0f,64.0f},{0.0f,0.0f},{1.0f,0.0f,0.0f,1.0f});
}

void BossHpUI::Update() {

	sprite_->scale_.x = static_cast<float>(currentHp_) / static_cast<float>(maxHp_);

	// 更新処理
	sprite_->Update();
}