#include"BossHpUI.h"
#include"EasingManager.h"
#include"FPSCounter.h"
#include"GameParamEditor.h"
using namespace GameEngine;

void BossHpUI::Initialize(const int32_t& maxHp) {

	maxHp_ = maxHp;
	currentHp_ = maxHp_;

	// 初期化
	sprite_ = Sprite::Create(position_, size_, { 0.0f,0.0f }, { 1.0f,0.0f,0.0f,1.0f });

	effectSprite_ = Sprite::Create(position_, size_, { 0.0f,0.0f }, { 0.8f,0.8f,0.0f,1.0f });

	// 後ろのフレーム
	frameSprite_ = Sprite::Create(position_, size_, { 0.0f,0.0f }, { 0.0f,0.0f,0.0f,1.0f });

	// ボスの名前UI
	bossNameSprite_ = Sprite::Create(bossNamePosition_, bossNameSize_, { 0.5f,0.5f }, { 1.0f,1.0f,1.0f,1.0f });

#ifdef _DEBUG
	RegisterBebugParam();
#endif
	ApplyDebugParam();
}

void BossHpUI::Update() {
#ifdef _DEBUG
	ApplyDebugParam();
#endif

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

void BossHpUI::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "HpSize", size_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "HpPosition", position_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "BossNameSize", bossNameSize_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "BossNamePosition", bossNamePosition_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "HpColor", sprite_->color_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "HpFrameColor", frameSprite_->color_);
}

void BossHpUI::ApplyDebugParam() {
	size_ = GameParamEditor::GetInstance()->GetValue<Vector2>(kGroupName_, "HpSize");
	position_ = GameParamEditor::GetInstance()->GetValue<Vector2>(kGroupName_, "HpPosition");
	bossNameSize_ = GameParamEditor::GetInstance()->GetValue<Vector2>(kGroupName_, "BossNameSize");
	bossNamePosition_ = GameParamEditor::GetInstance()->GetValue<Vector2>(kGroupName_, "BossNamePosition");

	// カラーはエディタから取得したRGBを反映しつつ、現在のアルファを維持する
	Vector4 hpColor = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "HpColor");
	Vector4 frameColor = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "HpFrameColor");
	float currentHpAlpha = sprite_->color_.w;
	float currentFrameAlpha = frameSprite_->color_.w;
	sprite_->color_ = { hpColor.x, hpColor.y, hpColor.z, currentHpAlpha };
	frameSprite_->color_ = { frameColor.x, frameColor.y, frameColor.z, currentFrameAlpha };

	sprite_->position_ = position_;
	sprite_->size_ = size_;
	effectSprite_->position_ = position_;
	effectSprite_->size_ = size_;
	frameSprite_->position_ = position_;
	frameSprite_->size_ = size_;

	bossNameSprite_->position_ = bossNamePosition_;
	bossNameSprite_->size_ = bossNameSize_;

	sprite_->Update();
	effectSprite_->Update();
	frameSprite_->Update();
	bossNameSprite_->Update();
}