#pragma once
#include<array>
#include"WorldTransform.h"

#include"EnemyRushParticle.h"

class EnemyRushEffect {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(const uint32_t& texture);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix);

public:

	/// <summary>
	/// 親を設定
	/// </summary>
	/// <param name="parent"></param>
	void SetParent(GameEngine::WorldTransform* parent) {
		for (size_t i = 0; i < worldTransforms_.size(); ++i) {
			worldTransforms_[i].SetParent(parent);
		}
	}

	// パーティクルの設定
	void SetActiveParticle(const bool& isActive, const Vector3& pos, const Vector3& velocity) {
		// パーティクルの設定
		enemyRushParticle_->SetIsLoop(isActive);
		if (isActive) {
			enemyRushParticle_->SetEmitterPos(pos, velocity);
		}
	}

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	std::array<GameEngine::WorldTransform, 3>& GetWorldTransforms() { return worldTransforms_; }

	// パーティクル
	std::unique_ptr<EnemyRushParticle> enemyRushParticle_;

private:
	// ワールド行列
	std::array<GameEngine::WorldTransform,3> worldTransforms_;

	// 回転速度
	float rotateSpeed_ = 50.0f;

	// 有効フラグ
	bool isActive_ = false;
};