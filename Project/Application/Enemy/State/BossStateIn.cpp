#include"BossStateIn.h"

using namespace GameEngine;

BossStateIn::BossStateIn(BossContext& context) : bossContext_(context) {
	
}

void BossStateIn::Enter() {

}
	
void BossStateIn::Update() {

	bossContext_.bossStateRequest_ = BossState::Battle;

}

void BossStateIn::Exit() {

}

void BossStateIn::RegisterBebugParam() {

}

void BossStateIn::ApplyDebugParam() {

}