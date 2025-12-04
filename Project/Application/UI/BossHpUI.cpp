#include"BossHpUI.h"
#include"EasingManager.h"
#include"FPSCounter.h"
using namespace GameEngine;

void BossHpUI::Initialize(const uint32_t& maxHp) {

	maxHp;
	maxHp_ = 2;
	currentHp_ = maxHp_;

	// 初期化
	sprite_ = Sprite::Create({ 320.0f,32.0f }, { 640.0f,48.0f }, { 0.0f,0.0f }, { 1.0f,0.0f,0.0f,1.0f });

	effectSprite_ = Sprite::Create({ 320.0f,32.0f }, { 640.0f,48.0f }, { 0.0f,0.0f }, { 0.5f,0.0f,0.0f,1.0f });
}

void BossHpUI::Update() {

	preScaleX_ = sprite_->scale_.x;

	sprite_->scale_.x = static_cast<float>(currentHp_) / static_cast<float>(maxHp_);

	// 位置を設定する
	if (sprite_->scale_.x != preScaleX_) {
		points_.push_back(Point(preScaleX_, sprite_->scale_.x, 0.0f));
	}

	// 演出の更新処理
	EffectUpdate();

	// 更新処理
	sprite_->Update();
	effectSprite_->Update();
}

void BossHpUI::EffectUpdate() {

	if (points_.size() != 0) {

		// 先頭要素を取得する
		auto& point = *points_.begin();

		point.timer += FpsCounter::deltaTime / maxTime_;

		effectSprite_->scale_.x = Lerp(point.startScale, point.endScale, EaseIn(point.timer));

		if (point.timer >= 1.0f) {
			effectSprite_->scale_.x = point.endScale;

			// 削除
			points_.pop_front();
		}
	}
}