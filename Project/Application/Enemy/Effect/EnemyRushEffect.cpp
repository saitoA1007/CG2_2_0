#include"EnemyRushEffect.h"

#include"FPSCounter.h"
using namespace GameEngine;

void EnemyRushEffect::Initialize(const uint32_t& texture) {
	for (size_t i = 0; i < worldTransforms_.size(); ++i) {
		float index = static_cast<float>(i);
		float scale = -index * 0.4f + 1.5f;
		worldTransforms_[i].Initialize({ { scale, scale, scale},{0.0f,0.0f,0.0f},{0.0f,0.5f,0.5f + index * 0.5f}});
	}

	// 突進パーティクル
	enemyRushParticle_ = std::make_unique<EnemyRushParticle>();
	enemyRushParticle_->Initialize(texture);
}

void EnemyRushEffect::Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix) {

	// 更新処理
	for (size_t i = 0; i < worldTransforms_.size(); ++i) {
		worldTransforms_[i].transform_.rotate.z += rotateSpeed_ * FpsCounter::deltaTime;

		if (worldTransforms_[i].transform_.rotate.z > 3.2f) {
			worldTransforms_[i].transform_.rotate.z = 0.0f;
		}
		// 行列を更新する
		worldTransforms_[i].UpdateTransformMatrix();
	}

	// パーティクルの更新処理
	enemyRushParticle_->Update(cameraMatrix, viewMatrix);
}