#include"BossStateIn.h"
#include"Application/Enemy/BossState.h"
#include"FPSCounter.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
#include<numbers>
using namespace GameEngine;

BossStateIn::BossStateIn(BossContext& context) : bossContext_(context) {

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
	ApplyDebugParam();
#else
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void BossStateIn::Enter() {
	// 自分の初期位置を設定
	bossContext_.worldTransform->transform_.translate = startPos_;

	// 正面を向くように
	bossContext_.worldTransform->transform_.rotate.y = std::numbers::pi_v<float>;

	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);
	bossContext_.animationTimer = 0.0f;

}

void BossStateIn::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	if (isMove_) {

		timer_ += FpsCounter::deltaTime / InTime_;

		// 縦移動
		float posY = 0.0f;
		float totalCycle = timer_ * static_cast<float>(cycleCount_);
		float localTimer = std::fmodf(totalCycle, 1.0f);

		if (localTimer <= 0.5f) {
			float t = localTimer / 0.5f;
			posY = Lerp(0.0f, moveHeight_, EaseInOut(t));
		} else {
			float t = (localTimer - 0.5f) / 0.5f;
			posY = Lerp(moveHeight_, 0.0f, EaseInOut(t));
		}

		bossContext_.worldTransform->transform_.translate = Lerp(startPos_, endPos_, EaseInOut(timer_));
		bossContext_.worldTransform->transform_.translate.y += posY;

		// アニメーション
		bossContext_.animationTimer += FpsCounter::deltaTime;
		if (bossContext_.animationTimer >= 1.0f) {
			bossContext_.animationTimer = 0.0f;
		}

		// 地面に付いたらバトル状態へ変更する
		if (timer_ >= 1.0f) {
			bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Scream)]["Scream"]);
			isMove_ = false;
			timer_ = 0.0f;
		}
	} else {

		timer_ += FpsCounter::deltaTime / waitTime_;

		// アニメーション
		bossContext_.animationTimer = timer_;

		if(timer_ >= 1.0f){
			bossContext_.bossStateRequest_ = BossState::Battle;
		}
	}	
}

void BossStateIn::Exit() {

}

void BossStateIn::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem(kGroupName, "StartPos", startPos_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "EndPos", endPos_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "InTime", InTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "WaitTime", waitTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "MoveHeight", moveHeight_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "CycleCount", cycleCount_);
}

void BossStateIn::ApplyDebugParam() {
	startPos_ = GameParamEditor::GetInstance()->GetValue<Vector3>(kGroupName, "StartPos");
	endPos_ = GameParamEditor::GetInstance()->GetValue<Vector3>(kGroupName, "EndPos");
	InTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "InTime");
	waitTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "WaitTime");
	moveHeight_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "MoveHeight");
	cycleCount_ = GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupName, "CycleCount");
}