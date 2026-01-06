#pragma once
#include"ParticleSystem/ParticleBehavior.h"

class PlayerAttackEffect {
public:

	void Initialize(const Vector3& emitPos);

	void Update(const Matrix4x4& worldMatrix, const Matrix4x4& viewMatrix);

	bool IsFinished() { return isFinished_; }

	uint32_t GetEffectNumInstance() { return attackEffectParticle_->GetCurrentNumInstance(); }
	GameEngine::WorldTransforms* GetEffectWorldTransforms() { return attackEffectParticle_->GetWorldTransforms(); }

	uint32_t GetAccentEffectNumInstance() { return attackAccentEffectParticle_->GetCurrentNumInstance(); }
	GameEngine::WorldTransforms* GetAccentEffectWorldTransforms() { return attackAccentEffectParticle_->GetWorldTransforms(); }

private:

	bool isFinished_ = false;

	// ダメージを与えた時のパーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> attackEffectParticle_;
	std::unique_ptr<GameEngine::ParticleBehavior> attackAccentEffectParticle_;

	float timer_ = 0.0f;
	float maxTime_ = 2.0f;
};