#include"RandMove.h"
#include"FPSCounter.h"

using namespace GameEngine;

RandMove::RandMove(BossContext& context) : bossContext_(context) {

}

void RandMove::Initialize() {
	timer_ = 0.0f;
	isFinished_ = false;
	phase_ = Phase::In;
}

void RandMove::Update() {
	timer_ += FpsCounter::deltaTime / maxTime_;

	if (timer_ >= 1.0f) {
		isFinished_ = true;
	}

	switch (phase_)
	{
	case RandMove::Phase::In:
		break;


	case RandMove::Phase::Move:
		break;


	case RandMove::Phase::Out:
		break;
	}
}

void RandMove::Finalize() {

}

void RandMove::RegisterDebugParam() {

}

void RandMove::ApplyDebugParam() {

}