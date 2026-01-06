#include"MaskScreenUI.h"
#include"EasingManager.h"
#include"FPSCounter.h"
using namespace GameEngine;

void MaskScreenUI::Initialize(GameEngine::InputCommand* inputCommand) {
	// 入力処理を取得
	inputCommand_ = inputCommand;

	upMaskSprite_ = Sprite::Create({ 0.0f, -maskSize_ }, { 1280.0f, maskSize_ }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f });

	downMaskSprite_ = Sprite::Create({ 0.0f, 720.0f}, { 1280.0f, maskSize_ }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f });
}

void MaskScreenUI::Update() {

	// 切り替え
	if (inputCommand_->IsCommandActive("CameraLockOn")) {

		if (isActive_) { return; }
		isActive_ = true;

		if (isMask_) {
			isMask_ = false;
			// 上
			upStartPosY_ = 0.0f;
			upEndPosY_ = -maskSize_;
			// 下
			downStartPosY_ = 720.0f - maskSize_;
			downEndPosY_ = 720.0f;
		} else {
			isMask_ = true;
			// 上
			upStartPosY_ = -maskSize_;
			upEndPosY_ = 0.0f;
			// 下
			downStartPosY_ = 720.0f;
			downEndPosY_ = 720.0f - maskSize_;
		}
	}

	if (isActive_) {
		timer_ += FpsCounter::deltaTime / maxTime_;

		upMaskSprite_->position_.y = Lerp(upStartPosY_, upEndPosY_, EaseInOut(timer_));
		downMaskSprite_->position_.y = Lerp(downStartPosY_, downEndPosY_, EaseInOut(timer_));

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			isActive_ = false;
			upMaskSprite_->position_.y = upEndPosY_;
			downMaskSprite_->position_.y = downEndPosY_;
		}

		// 画像の更新
		downMaskSprite_->Update();
		upMaskSprite_->Update();
	}
}