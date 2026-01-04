#include"PlayerHpUI.h"

using namespace GameEngine;

void PlayerHpUI::Initialize(const int32_t& maxHp, GameEngine::TextureManager* textureManager) {
	maxHp_ = maxHp;
	currentHp_ = maxHp_;

	// メモリを確保しておく
	hpSprites_.clear();
	hpSprites_.reserve(static_cast<size_t>(maxHp_));

	for (int i = 0; i < maxHp_; ++i) {
		Vector2 pos = { startPos_.x + i * (spriteSize_.x + spacing_), startPos_.y };
		auto sprite = Sprite::Create(pos, spriteSize_, { 0.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f });
		hpSprites_.push_back(std::move(sprite));
	}

	// hp画像を取得
	hpGH_ = textureManager->GetHandleByName("PlayerHP.png");
}

void PlayerHpUI::Update() {

	for (int i = 0; i < maxHp_; ++i) {
		auto& sprite = hpSprites_[static_cast<size_t>(i)];
		if (i < currentHp_) {
			// 残りhpは白
			sprite->color_ = { 1.0f,1.0f,1.0f,1.0f };
		} else {
			// 減っている分は黒
			sprite->color_ = { 0.0f,0.0f,0.0f,1.0f };
		}
		// スプライトの更新処理
		sprite->Update();
	}
}