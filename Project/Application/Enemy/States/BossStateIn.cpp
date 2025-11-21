#include"BossStateIn.h"
#include"Application/Enemy/BossState.h"
#include"FPSCounter.h"
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
	bossContext_.worldTransform->transform_.translate.x = 10.0f;
	bossContext_.worldTransform->transform_.translate.z = 10.0f;
	bossContext_.worldTransform->transform_.translate.y = 20.0f;
}

void BossStateIn::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	// 落下する
	bossContext_.worldTransform->transform_.translate.y -= 10.0f * FpsCounter::deltaTime;

	// 地面に付いたらバトル状態へ変更する
	if (bossContext_.worldTransform->transform_.translate.y <= 6.0f) {
		bossContext_.bossStateRequest_ = BossState::Battle;
	}
}

void BossStateIn::Exit() {

}

void BossStateIn::RegisterBebugParam() {

}

void BossStateIn::ApplyDebugParam() {

}