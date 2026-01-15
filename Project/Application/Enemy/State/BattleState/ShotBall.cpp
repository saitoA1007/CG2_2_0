#include"ShotBall.h"
#include"FPSCounter.h"
#include"MyMath.h"
#include"EasingManager.h"
//#include"GameParamEditor.h"
using namespace GameEngine;

ShotBall::ShotBall(BossContext& context) : bossContext_(context) {

}

void ShotBall::Initialize() {
	phase_ = Phase::In;
	timer_ = 0.0f;
	isFinished_ = false;

	// 最初の位置を取得する
	startPosY_ = bossContext_.worldTransform->transform_.translate.y;
}

void ShotBall::Update() {

	//====================================================================
	// 
	// 弾を発射する時は浮くようにする
	// 
	//====================================================================

	switch (phase_)
	{
	case ShotBall::Phase::In: {

		timer_ += FpsCounter::deltaTime / inTime_;

		// 上に移動
		bossContext_.worldTransform->transform_.translate.y = Lerp(startPosY_, maxHeightPosY_, EaseOut(timer_));

		// 目標へのベクトルを求める
		Vector3 toTarget = bossContext_.targetPos - bossContext_.worldTransform->transform_.translate;
		toTarget = Normalize(toTarget);

		Vector3 targetRot = { 0, 0, 0 };
		// Y軸回転を取得
		targetRot.y = atan2f(toTarget.x, toTarget.z);
		Vector3 currentRot = bossContext_.worldTransform->transform_.rotate;

		// 最短距離の角度を取得
		float diffY = GetShortAngleY(targetRot.y - currentRot.y);

		// 回転
		currentRot.y += diffY * rotateSpeed_ * FpsCounter::deltaTime;

		bossContext_.worldTransform->transform_.rotate = currentRot;

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
	}

	case ShotBall::Phase::Throw:

		timer_ += FpsCounter::deltaTime / throwTime_;

		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			phase_ = Phase::Out;
		}
		break;


	case ShotBall::Phase::Out:

		timer_ += FpsCounter::deltaTime / outTime_;

		// 下に移動
		bossContext_.worldTransform->transform_.translate.y = Lerp(maxHeightPosY_, startPosY_, EaseIn(timer_));

		if (timer_ >= 1.0f) {
			isFinished_ = true;
		}
		break;
	}
}

void ShotBall::Finalize() {

}

void ShotBall::RegisterDebugParam() {

}

void ShotBall::ApplyDebugParam() {

}