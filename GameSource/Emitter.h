#pragma once
#include"EngineSource/3D/WorldTransform.h"
#include"EngineSource/3D/Model.h"

class ParticleEmitter {
public:

	struct Emitter {
		Transform transform; // エミッタのTransform
		uint32_t count; // 発生数
		float frequency; // 発生頻度
		float frequencyTime; // 頻度用時刻
	};

public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model"></param>
	/// <param name="textureHandle"></param>
	void Initialize(GameEngine::Model* model, const uint32_t& textureHandle);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="VPMatrix"></param>
	void Draw(const Matrix4x4 VPMatrix);

	bool IsCall() const { return isCall_; }

	Emitter GetEmitter() const { return emitter_; }

private:

	// エミッタ
	Emitter emitter_{};

	bool isCall_ = false;

	// モデル
	GameEngine::Model* model_;
	// ワールド行列
	GameEngine::WorldTransform worldTransform_;
	// テクスチャ
	uint32_t textureHandle_ = 0u;

	const float kDeltaTime = 1.0f / 60.0f;
};