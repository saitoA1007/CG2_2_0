#include"SceneTransition.h"
#include"EasingManager.h"
#include"FPSCounter.h"
#include"SpriteRenderer.h"
using namespace GameEngine;

void SceneTransition::Initialize() {

	// 初期化
	sprite_ = Sprite::Create({ 0.0f,0.0f }, { 1280.0f,720.0f }, { 0.0f,0.0f }, { 0.0f,0.0f,0.0f,0.0f });
}

void SceneTransition::Update() {
	if (!isActive_) {
		return;
	}

	timer_ += FpsCounter::deltaTime / maxTime_;

	// 演出タイプに応じた更新処理
	switch (type_) {
	case TransitionType::Fade:
		Fade();
		break;
	}

	// 演出の終了
	if (timer_ >= 1.0f) {
		isActive_ = false;
	}

	// 更新処理
	sprite_->Update();
}

void SceneTransition::Draw() {

	// 画像の描画前処理
	SpriteRenderer::PreDraw(RenderMode2D::Normal);

	// 遷移画像を表示
	SpriteRenderer::Draw(sprite_.get(), 0);
}

void SceneTransition::Start(TransitionType type, float maxTime) {
	isActive_ = true;
	type_ = type;
	maxTime_ = maxTime;
	timer_ = 0.0f;
	sprite_->color_.w = 0.0f;
}

void SceneTransition::Fade() {

	if (timer_ <= 0.5f) {
		float localT = timer_ / 0.5f;
		sprite_->color_.w = Lerp(0.0f, 1.0f, localT);
	} else {
		float localT = (timer_ - 0.5f) / 0.5f;
		sprite_->color_.w = Lerp(1.0f, 0.0f, localT);
	}
}