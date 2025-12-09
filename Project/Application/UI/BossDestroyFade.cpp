#include"BossDestroyFade.h"
#include"EasingManager.h"
#include"FPSCounter.h"
using namespace GameEngine;

void BossDestroyFade::Initialize() {
	// 初期化
	sprite_ = Sprite::Create({ 0.0f,0.0f }, { 1280.0f,720.0f }, { 0.0f,0.0f }, { 0.0f,0.0f,0.0f,0.0f });
	phase_ = Phase::In;
}

void BossDestroyFade::Update() {

	if (!isActive_) { return; }


	switch (phase_)
	{
	case BossDestroyFade::Phase::In:

		timer_ += FpsCounter::deltaTime / 0.5f;

		sprite_->color_.w = Lerp(0.0f, 1.0f, timer_);

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Wait;
			isMiddle_ = true;
		}
		break;

	case BossDestroyFade::Phase::Wait:

		timer_ += FpsCounter::deltaTime / 0.2f;

		sprite_->color_.w = Lerp(1.0f, 0.0f, timer_);

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Out;
			isMiddle_ = false;
		}
		break;

	case BossDestroyFade::Phase::Out:

		timer_ += FpsCounter::deltaTime / 0.5f;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			isActive_ = true;
		}
		break;
	}

	// 更新処理
	sprite_->Update();
}