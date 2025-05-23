#pragma once
#include"EngineSource/3D/WorldTransforms.h"
#include"EngineSource/3D/Model.h"

#include"RandomGenerator.h"

class Particle {
public:

	struct ParticleData {
		Transform transform;
		Vector3 velocity;
		Vector4 color;
		float lifeTime;
		float currentTime;
	};

public:

	Particle() = default;
	~Particle();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャ</param>
	void Initialize(GameEngine::Model* model,const uint32_t& textureHandle);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(const Matrix4x4& cameraMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="VPMatrix"></param>
	void Draw(const Matrix4x4 VPMatrix);

private:

	// ランダム生成器
	RandomGenerator randomGenerator_;

	// 平面モデル
	GameEngine::Model* planeModel_;

	// モデルを描画する数
	const uint32_t kNumMaxInstance = 10;
	// パーティクルデータ
	std::vector<ParticleData> particleDatas_;
	// ワールド行列
	GameEngine::WorldTransforms planeWorldTransforms_;
	// テクスチャ
	uint32_t textureHandle_ = 0u;

	// デルタタイム
	const float kDeltaTime = 1.0f / 60.0f;

	// 現在の生成数
	uint32_t numInstance_ = 0;

	// ビルボードを有効にするかのフラグ
	bool useBillboard_ = false;

private:

	/// <summary>
	/// パーティクル生成関数
	/// </summary>
	ParticleData MakeNewParticle();
};
