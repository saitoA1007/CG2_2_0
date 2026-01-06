#include"EnemyDestroyEffect.h"
#include"FPSCounter.h"
using namespace GameEngine;

void EnemyDestroyEffect::Initialize() {
	// 攻撃演出
	destroyParticle_ = std::make_unique<ParticleBehavior>();
	destroyParticle_->Initialize("EnemyDestroy", 16);
}

void EnemyDestroyEffect::Update(const Matrix4x4& worldMatrix, const Matrix4x4& viewMatrix) {
	if (!isActive_) { return; }

	timer_ += FpsCounter::deltaTime / maxTime_;

	// 時間が経過したら終了する
	if (timer_ >= 1.0f) {
		isFinished_ = true;
	}

	destroyParticle_->Update(worldMatrix, viewMatrix);
}