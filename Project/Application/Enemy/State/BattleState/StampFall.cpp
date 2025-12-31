#include"StampFall.h"
#include"FPSCounter.h"
#include"EasingManager.h"
using namespace GameEngine;

StampFall::StampFall(BossContext& context) : bossContext_(context) {

}

void StampFall::Initialize() {
	phase_ = Phase::Rise;
	timer_ = 0.0f;
	isFinished_ = false;
}

void StampFall::Update() {

	switch (phase_)
	{
	case StampFall::Phase::Rise:

		timer_ += FpsCounter::deltaTime / riseTime_;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Move;
		}
		break;


	case StampFall::Phase::Move:

		timer_ += FpsCounter::deltaTime / moveTime_;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Fall;
		}
		break;


	case StampFall::Phase::Fall:

		timer_ += FpsCounter::deltaTime / fallTime_;

		if (timer_ >= 1.0f) {
			isFinished_ = true;
		}
		break;
	}
}

void StampFall::Finalize() {

}

void StampFall::RegisterBebugParam() {

}

void StampFall::ApplyDebugParam() {

}