#include"StampFall.h"
#include"FPSCounter.h"
#include"EasingManager.h"
#include"GameParamEditor.h"
using namespace GameEngine;

StampFall::StampFall(BossContext& context) : bossContext_(context) {

#ifdef USE_IMGUI
	RegisterBebugParam();
#endif 
	ApplyDebugParam();
}

void StampFall::Initialize() {
	phase_ = Phase::Rise;
	timer_ = 0.0f;
	isFinished_ = false;

	// 最初の位置を取得する
	startRisePosY_ = bossContext_.worldTransform->transform_.translate.y;

#ifdef USE_IMGUI
	ApplyDebugParam();
#endif
}

void StampFall::Update() {

	switch (phase_)
	{
	case StampFall::Phase::Rise:

		timer_ += FpsCounter::deltaTime / riseTime_;

		// 上に移動
		bossContext_.worldTransform->transform_.translate.y = Lerp(startRisePosY_, maxHeightPosY_,EaseOut(timer_));

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Move;
			bossContext_.worldTransform->transform_.translate.y = maxHeightPosY_;
			// 移動する最後の位置を設定
			endMovePos_ = bossContext_.targetPos;
			endMovePos_.y = maxHeightPosY_;
			// 移動する最初の位置を設定
			startMovePos_ = bossContext_.worldTransform->transform_.translate;
		}
		break;


	case StampFall::Phase::Move:

		timer_ += FpsCounter::deltaTime / moveTime_;

		// 目標の位置まで移動
		bossContext_.worldTransform->transform_.translate = Lerp(startMovePos_, endMovePos_, EaseInOut(timer_));

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Fall;
			bossContext_.worldTransform->transform_.translate = endMovePos_;
		}
		break;


	case StampFall::Phase::Fall:

		timer_ += FpsCounter::deltaTime / fallTime_;

		// 落下移動
		bossContext_.worldTransform->transform_.translate.y = Lerp(endMovePos_.y, startRisePosY_, EaseOutBounce(timer_));

		if (timer_ >= 1.0f) {
			bossContext_.worldTransform->transform_.translate.y = startRisePosY_;
			isFinished_ = true;
		}
		break;
	}
}

void StampFall::Finalize() {

}

void StampFall::RegisterBebugParam() {
	// 値の登録
	GameParamEditor::GetInstance()->AddItem(groupName_, "MaxHeightPosY", maxHeightPosY_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RiseTime", riseTime_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "MoveTime", moveTime_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "FallTime", fallTime_);
}

void StampFall::ApplyDebugParam() {
	// 値の適応
	maxHeightPosY_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "MaxHeightPosY");
	riseTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "RiseTime");
	moveTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "MoveTime");
	fallTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "FallTime");
}