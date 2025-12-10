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
	// 仕様に合わせて固定値を設定
	InTime_ = 3.0f;
	waitTime_ = 6.0f;
}

void BossStateIn::Enter() {
	// 自分の初期位置を設定
	bossContext_.worldTransform->transform_.translate = startPos_;

	// 初期回転を0度に設定
	bossContext_.worldTransform->transform_.rotate.y = 0.0f;

	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Appearance)]["Appearance_Animation_Rotate"]);
	bossContext_.animationTimer = 0.0f;

	// 移動開始
	isMove_ = true;
	timer_ = 0.0f;
}

void BossStateIn::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	if (isMove_) {
		// 正規化タイマー進行（0.0f〜1.0f）
		timer_ += FpsCounter::deltaTime / InTime_;
		float t = std::clamp(timer_, 0.0f, 1.0f);

		// 上昇（Y: 0 -> 8）EaseOutBack
		float riseY = EaseOutBack(0.0f, moveHeight_, t);
		bossContext_.worldTransform->transform_.translate = startPos_;
		bossContext_.worldTransform->transform_.translate.y = startPos_.y + riseY;

		// 回転（Y軸: 0deg -> 1800deg）EaseOutSine（度->ラジアンに変換）
		float targetDeg = EaseOutSine(0.0f, 1620.0f, t);
		float targetRad = targetDeg * (std::numbers::pi_v<float> / 180.0f);
		bossContext_.worldTransform->transform_.rotate.y = targetRad;

		// アニメーションタイマー更新（必要なら）
		bossContext_.animationTimer += FpsCounter::deltaTime;
		if (bossContext_.animationTimer >= 1.0f) { bossContext_.animationTimer = 0.0f; }

		// 終了判定
		if (timer_ >= 1.0f) {
			bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Appearance)]["Appearance_Animation_2nd"]);
			isMove_ = false;
			// 次フェーズへ向けてタイマーリセット
			timer_ = 0.0f;
		}
	} else {
		// 待機タイマー（正規化）
        timer_ += FpsCounter::deltaTime / waitTime_;
		bossContext_.animationTimer = timer_;

		if (timer_ >= waitTime_) {
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