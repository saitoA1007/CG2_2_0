#include"BossStateOut.h"
#include"FPSCounter.h"
using namespace GameEngine;

BossStateOut::BossStateOut(BossContext& context) : bossContext_(context) {
	
}

void BossStateOut::Enter() {

}

void BossStateOut::Update() {

	timer_ += FpsCounter::deltaTime / maxTime_;

	if (timer_ >= 1.0f) {
		bossContext_.isFinished_ = true;
	}
}

void BossStateOut::Exit() {

}

void BossStateOut::RegisterBebugParam() {

}

void BossStateOut::ApplyDebugParam() {

}