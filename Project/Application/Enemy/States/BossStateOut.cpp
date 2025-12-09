#include"BossStateOut.h"
#include"Application/Enemy/BossState.h"
#include"FPSCounter.h"
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

	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);
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

}

void BossStateOut::ApplyDebugParam() {

}