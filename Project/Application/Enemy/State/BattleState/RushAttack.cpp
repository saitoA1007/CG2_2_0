#include"RushAttack.h"
#include"FPSCounter.h"
#include"MyMath.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
#include"AudioManager.h"
using namespace GameEngine;

RushAttack::RushAttack(BossContext& context) : bossContext_(context) {
#ifdef USE_IMGUI
	RegisterBebugParam();
#endif 
	ApplyDebugParam();

	rushSH_ = AudioManager::GetInstance().GetHandleByName("enemyDush.mp3");
}

void RushAttack::Initialize() {
	timer_ = 0.0f;
	isFinished_ = false;
	phase_ = Phase::In;

#ifdef USE_IMGUI
	ApplyDebugParam();
#endif
}

void RushAttack::Update() {
	
	switch (phase_)
	{
	case RushAttack::Phase::In: {

		timer_ += FpsCounter::deltaTime / InTime_;

		// 目標へのベクトルを求める
		Vector3 toTarget = bossContext_.targetPos - bossContext_.worldTransform->transform_.translate;
		toTarget = Normalize(toTarget);

		Vector3 targetRot = { 0, 0, 0 };
		// Y軸回転を取得
		targetRot.y = atan2f(toTarget.x, toTarget.z);
		Vector3 currentRot = bossContext_.worldTransform->transform_.rotate;

		// 最短距離の角度を取得
		float diffY = GetShortAngleY(targetRot.y - currentRot.y);

		// 回転
		currentRot.y += diffY * rotateSpeed_ * FpsCounter::deltaTime;

		bossContext_.worldTransform->transform_.rotate = currentRot;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Move;
			dir_ = MakeDirVector({ 0,0,1 }, { 0,bossContext_.worldTransform->transform_.rotate.y,0 });

			// 突進する音
			AudioManager::GetInstance().Play(rushSH_, 0.3f, false);
		}
		break;
	}


	case RushAttack::Phase::Move: {
		timer_ += FpsCounter::deltaTime / moveTime_;

		// 速度
		float speed = Lerp(startRushSpeed_, endRushSpeed_, EaseIn(timer_));
		Vector3 velocity = dir_ * speed;

		// 移動
		bossContext_.worldTransform->transform_.translate += velocity * FpsCounter::deltaTime;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Out;
		}
		break;
	}


	case RushAttack::Phase::Out:
		timer_ += FpsCounter::deltaTime / OutTime_;

		if (timer_ >= 1.0f) {
			isFinished_ = true;
		}

		break;
	}
}

void RushAttack::Finalize() {

}

void RushAttack::RegisterBebugParam() {
	// 値の登録
	GameParamEditor::GetInstance()->AddItem(groupName_, "InTime", InTime_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "MoveTime", moveTime_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "OutTime", OutTime_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RotateSpeed", rotateSpeed_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "StartRushSpeed", startRushSpeed_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "EndRushSpeed", endRushSpeed_);
}

void RushAttack::ApplyDebugParam() {
	InTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "InTime");
	moveTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "MoveTime");
	OutTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "OutTime");
	rotateSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "RotateSpeed");
	startRushSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "StartRushSpeed");
	endRushSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "EndRushSpeed");
}