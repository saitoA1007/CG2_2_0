#include"BossStateOut.h"
#include"Application/Enemy/BossState.h"
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
}

void BossStateOut::Exit() {

}

void BossStateOut::RegisterBebugParam() {

}

void BossStateOut::ApplyDebugParam() {

}