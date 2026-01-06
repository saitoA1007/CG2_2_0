#pragma once
#include"ParticleSystem/ParticleBehavior.h"

class PlayerDestroyEffect {
public:

	void Initialize();

	void Update(const Matrix4x4& worldMatrix, const Matrix4x4& viewMatrix);

	// 発生
	void SetEmitPos(const Vector3& pos) {
		isActive_ = true;
		destroyParticle_->Emit(pos);
	}

	bool IsFinished() { return isFinished_; }

	uint32_t GetNumInstance() { return destroyParticle_->GetCurrentNumInstance(); }
	GameEngine::WorldTransforms* GetWorldTransforms() { return destroyParticle_->GetWorldTransforms(); }

private:

	bool isFinished_ = false;

	bool isActive_ = false;

	std::unique_ptr<GameEngine::ParticleBehavior> destroyParticle_;

	float timer_ = 0.0f;
	float maxTime_ = 2.0f;
};