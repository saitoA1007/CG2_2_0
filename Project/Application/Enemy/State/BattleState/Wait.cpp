#include"Wait.h"
#include"FPSCounter.h"

using namespace GameEngine;

Wait::Wait(BossContext& context) : bossContext_(context) {
	
}

void Wait::Initialize() {
	timer_ = 0.0f;
	isFinished_ = false;
}

void Wait::Update() {
	timer_ += FpsCounter::deltaTime / maxTime_;

	if (timer_ >= 1.0f) {
		isFinished_ = true;
	}
}

void Wait::Finalize() {

}

void Wait::RegisterBebugParam() {

}

void Wait::ApplyDebugParam() {

}