#include"RotateAttack.h"
#include"FPSCounter.h"
#include"EasingManager.h"
#include"GameParamEditor.h"
using namespace GameEngine;

RotateAttack::RotateAttack(BossContext& context) : bossContext_(context) {
#ifdef USE_IMGUI
	RegisterBebugParam();
#endif 
	ApplyDebugParam();
}

void RotateAttack::Initialize() {
	timer_ = 0.0f;
	isFinished_ = false;
	phase_ = Phase::In;

#ifdef USE_IMGUI
	ApplyDebugParam();
#endif
}

void RotateAttack::Update() {
	
	switch (phase_)
	{
	case RotateAttack::Phase::In:
		timer_ += FpsCounter::deltaTime / inTime_;

		// 回転速度
		rotSpeed_ = Lerp(0.0f, maxRotSpeed_, timer_);
		// 回転
		bossContext_.worldTransform->transform_.rotate.y += rotSpeed_ * FpsCounter::deltaTime;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Move;
			rotSpeed_ = maxRotSpeed_;
		}
		break;


	case RotateAttack::Phase::Move:
		timer_ += FpsCounter::deltaTime / moveTime_;

		// 回転
		bossContext_.worldTransform->transform_.rotate.y += rotSpeed_ * FpsCounter::deltaTime;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Out;
		}
		break;


	case RotateAttack::Phase::Out:
		timer_ += FpsCounter::deltaTime / outTime_;

		// 回転速度
		rotSpeed_ = Lerp(maxRotSpeed_, 0.0f, timer_);
		// 回転
		bossContext_.worldTransform->transform_.rotate.y += rotSpeed_ * FpsCounter::deltaTime;

		if (timer_ >= 1.0f) {
			rotSpeed_ = 0.0f;
			isFinished_ = true;
		}
		break;
	}

}

void RotateAttack::Finalize() {

}

void RotateAttack::RegisterBebugParam() {
	// 値の登録
	GameParamEditor::GetInstance()->AddItem(groupName_, "InTime", inTime_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "MoveTime", moveTime_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "OutTime", outTime_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "MaxRotateSpeed", maxRotSpeed_);
}

void RotateAttack::ApplyDebugParam() {
	inTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "InTime");
	moveTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "MoveTime");
	outTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "OutTime");
	maxRotSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "MaxRotateSpeed");
}