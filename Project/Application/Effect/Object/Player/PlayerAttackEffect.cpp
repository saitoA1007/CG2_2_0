#include"PlayerAttackEffect.h"
#include"FPSCounter.h"
using namespace GameEngine;

void PlayerAttackEffect::Initialize(const Vector3& emitPos) {
	// 攻撃演出
	attackEffectParticle_ = std::make_unique<ParticleBehavior>();
	attackEffectParticle_->Initialize("PlayerAttackEffect", 32);
	// 攻撃演出のアクセント
	attackAccentEffectParticle_ = std::make_unique<ParticleBehavior>();
	attackAccentEffectParticle_->Initialize("PlayerAttackAccentEffect", 32);

	// パーティクルの発生
	attackEffectParticle_->Emit(emitPos);
	attackAccentEffectParticle_->Emit(emitPos);
}

void PlayerAttackEffect::Update(const Matrix4x4& worldMatrix,const Matrix4x4& viewMatrix) {

	timer_ += FpsCounter::deltaTime / maxTime_;

	// 時間が経過したら終了する
	if (timer_ >= 1.0f) {
		isFinished_ = true;
	}

	// 攻撃演出の更新処理
	attackEffectParticle_->Update(worldMatrix, viewMatrix);
	attackAccentEffectParticle_->Update(worldMatrix, viewMatrix);

}