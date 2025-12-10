#include"BossStateOut.h"
#include"Application/Enemy/BossState.h"
#include"FPSCounter.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
#include<numbers>
using namespace GameEngine;

BossStateOut::BossStateOut(BossContext& context) : bossContext_(context) {

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
#endif
	// 値を適応させる
	ApplyDebugParam();
}

void BossStateOut::Enter() {
	bossContext_.isDestroyEffect = true;

	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Death)]["アーマチュア"]);
	bossContext_.animationTimer = 0.0f;
}

void BossStateOut::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif
	if (cout_ == 1) {
		bossContext_.isDestroyEffect = false;
	}
	if (cout_ <= 2) {
		cout_++;
	}

	// 動かす
	if (bossContext_.isResetPos) {
		if (!isSet_) {
			isSet_ = true;
			isActive_ = true;

			// 座標を設定
			bossContext_.worldTransform->transform_.translate = { 0.0f,0.0f,0.0f };
			bossContext_.worldTransform->transform_.rotate = { 0.0f,std::numbers::pi_v<float>,0.0f };
		}
	}

	if (!isActive_) { return; }

	timer_ += FpsCounter::deltaTime / maxTime_;

	float swayFade = 1.0f - timer_;

	float timeValue = timer_ * swaySpeed_ + swayPhase_;
	float swayOffsetX = std::sinf(timeValue) * swayWeithX_ * swayFade;
	float swayOffsetZ = std::sinf(timeValue * 2.0f) * swayWeithZ_ * swayFade;

	Vector3 basePos;
	basePos.x = 0.0f;
	basePos.y = Lerp(startPosY_, endPosY_, EaseInOut(timer_));
	basePos.z = 0.0f;

	// 縦移動
	float posY = 0.0f;
	float totalCycle = timer_ * 3.0f;
	float localTimer = std::fmodf(totalCycle, 1.0f);

	if (localTimer <= 0.5f) {
		float t = localTimer / 0.5f;
		posY = Lerp(0.0f, cycleHeight_, EaseInOut(t));
	} else {
		float t = (localTimer - 0.5f) / 0.5f;
		posY = Lerp(cycleHeight_, 0.0f, EaseInOut(t));
	}

	basePos.y += posY;

	bossContext_.worldTransform->transform_.translate = basePos + Vector3(swayOffsetX, 0.0f, swayOffsetZ);

	// アニメーションを遷移
	bossContext_.animationTimer = timer_;

	if (timer_ >= 1.0f) {
		// ボスの全ての処理が終了
		bossContext_.isFinished_ = true;
		isActive_ = false;
	}
}

void BossStateOut::Exit() {

}

void BossStateOut::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem(kGroupName, "MaxTime", maxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "StartPosY", startPosY_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "EndPosY", endPosY_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "SwaySpeed", swaySpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "SwayPhase", swayPhase_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "SwayWeithX", swayWeithX_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "SwayWeithZ", swayWeithZ_);
	GameParamEditor::GetInstance()->AddItem(kGroupName, "CycleHeight", cycleHeight_);
}

void BossStateOut::ApplyDebugParam() {
	maxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "MaxTime");
	startPosY_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "StartPosY");
	endPosY_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "EndPosY");
	swaySpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "SwaySpeed");
	swayPhase_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "SwayPhase");
	swayWeithX_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "SwayWeithX");
	swayWeithZ_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "SwayWeithZ");
	cycleHeight_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName, "CycleHeight");
}