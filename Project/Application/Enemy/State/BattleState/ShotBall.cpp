#include"ShotBall.h"
#include"FPSCounter.h"

using namespace GameEngine;

ShotBall::ShotBall(BossContext& context) : bossContext_(context) {

}

void ShotBall::Initialize() {
	phase_ = Phase::In;
	timer_ = 0.0f;
	isFinished_ = false;
}

void ShotBall::Update() {

	switch (phase_)
	{
	case ShotBall::Phase::In:

		timer_ += FpsCounter::deltaTime / inTime_;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Throw;
		}
		break;


	case ShotBall::Phase::Throw:

		timer_ += FpsCounter::deltaTime / throwTime_;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Out;
		}
		break;


	case ShotBall::Phase::Out:

		timer_ += FpsCounter::deltaTime / outTime_;

		if (timer_ >= 1.0f) {
			isFinished_ = true;
		}
		break;
	}
}

void ShotBall::Finalize() {

}

void ShotBall::RegisterBebugParam() {

}

void ShotBall::ApplyDebugParam() {

}