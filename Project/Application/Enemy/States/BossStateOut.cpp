#include"BossStateOut.h"
#include"Application/Enemy/BossState.h"
using namespace GameEngine;

BossStateOut::BossStateOut(BossContext& context) : bossContext_(context) {

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
	ApplyDebugParam();
#else
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void BossStateOut::Enter() {

}

void BossStateOut::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

}

void BossStateOut::Exit() {

}

void BossStateOut::RegisterBebugParam() {

}

void BossStateOut::ApplyDebugParam() {

}