#pragma once
#include<array>
#include"WorldTransform.h"

class BreakEffect {
public:

	struct ParticleData {
		Vector3 velocity = {0.0f,0.0f,0.0f};
		float timer = 0.0f;
		float maxTime = 0.2f;
		Vector3 scale = {};
	};

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(const Vector3& pos);

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

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	std::array<GameEngine::WorldTransform, 3>& GetWorldTransforms() { return worldTransforms_; }

	bool IsFinished()const { return isFinished_; }

private:
	// ワールド行列
	std::array<GameEngine::WorldTransform, 3> worldTransforms_;

	std::array< ParticleData, 3> particleDatas_;

	bool isFinished_ = false;
};