#include"ShotBall.h"
#include"FPSCounter.h"
#include"MyMath.h"
using namespace GameEngine;

ShotBall::ShotBall(BossContext& context) : bossContext_(context) {

}

void ShotBall::Initialize() {
	phase_ = Phase::In;
	timer_ = 0.0f;
	isFinished_ = false;
}

void ShotBall::Update() {

	//====================================================================
	// 
	// 弾を発射する時は浮くようにする
	// 
	//====================================================================

	switch (phase_)
	{
	case ShotBall::Phase::In:

		timer_ += FpsCounter::deltaTime / inTime_;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Throw;

			// 岩を発射
			ProjectileSpwanPrams param;
			param.type = ProjectileType::Rock;
			param.pos = bossContext_.worldTransform->transform_.translate;
			param.pos.y += 5.0f;
			param.dir = Normalize(bossContext_.targetPos - param.pos);
			bossContext_.projectileManager->AddProjectile(param);
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