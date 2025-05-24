#pragma once
#include<list>
#include"EngineSource/3D/WorldTransforms.h"
#include"EngineSource/3D/Model.h"
#include"EngineSource/Math/Geometry.h"

#include"RandomGenerator.h"

#include"Emitter.h"

class Particle {
public:

	struct AccelerationField {
		Vector3 acceleration; // 加速度
		AABB area; // 範囲

	};

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

	/// <summary>
	/// エミッタの設定
	/// </summary>
	/// <param name="emitter"></param>
	void SetEmitter(ParticleEmitter* emitter) { particleEmitter_ = emitter; }

private:

	// エミッタのクラス
	ParticleEmitter* particleEmitter_;

	// ランダム生成器
	RandomGenerator randomGenerator_;

	// 平面モデル
	GameEngine::Model* planeModel_;

	// モデルを描画する数
	const uint32_t kNumMaxInstance = 100;
	// パーティクルデータ
	std::list<ParticleData> particleDatas_;
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

	// 場を有効にするかのフラグ
	bool enableField_ = false;

	// 場の設定
	AccelerationField accelerationField_;

private:

	/// <summary>
	/// パーティクル生成関数
	/// </summary>
	ParticleData MakeNewParticle(const Vector3& translate);

	std::list<ParticleData> Emit(const ParticleEmitter::Emitter& emitter);
};
