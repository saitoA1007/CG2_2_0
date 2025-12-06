#pragma once
#include"WorldTransforms.h"

class EnemyRushParticle {
public:

	struct ParticleData {
		Transform transform; // トラスフォーム
		Vector3 velocity;    // 速度
		Vector4 color;       // 色
		float lifeTime;      // 生存時間
		float currentTime;   // 現在の時間
		uint32_t textureHandle = 0; // 使用するテクスチャ
	};

public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="texture"></param>
	void Initialize(const uint32_t& texture);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix);

public:

	/// <summary>
	/// 発生位置を設定
	/// </summary>
	/// <param name="pos"></param>
	void SetEmitterPos(const Vector3& pos,const Vector3& velocity) { 
		emitterPos_ = pos; 
		baseVelocity_ = velocity;
	}

	// ループの設定
	void SetIsLoop(const bool& isLoop) { isLoop_ = isLoop; }

	/// <summary>
	/// 行列のデータを取得
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransforms* GetWorldTransforms() const { return worldTransforms_.get(); }

	/// <summary>
	/// 現在の数
	/// </summary>
	/// <returns></returns>
	uint32_t GetCurrentNumInstance() const { return numInstance_; }

	// 速度を設定する
	void SetVelocity(const Vector3& velocity) { baseVelocity_ = velocity; }

private:

	// 最大描画数
	static inline const uint32_t kNumMaxInstance = 64;

	// 画像を取得
	uint32_t particleGH_ = 0u;

	// ワールド行列
	std::unique_ptr<GameEngine::WorldTransforms> worldTransforms_;

	// 現在の発生数
	uint32_t numInstance_ = 0;

	// パーティクルデータ
	std::vector<ParticleData> particleDatas_;

	// パーティクルの発生位置を取得
	Vector3 emitterPos_ = { 0.0f,0.0f,0.0f };

	// 時間
	float timer_ = 0.0f;

	float coolTime_ = 0.2f;

	// ループの判定
	bool isLoop_ = true;

	// 速度
	Vector3 baseVelocity_ = {};

	float lifeTime_ = 2.0f;
	std::string name_ = "EnemyRushParticle";

	// サイズ
	float scaleYMin_ = 1.0f;
	float scaleYMax_ = 1.0f;
	// 速度
	float speedMin_ = 10.0f;
	float speedMax_ = 10.0f;
	// 範囲
	float spawnPosMin_ = 1.0f;
	float spawnPosMax_ = 1.0f;

	// 発生する数
	uint32_t spawnCount_ = 1;

private:

	/// <summary>
	/// パーティクル生成装置
	/// </summary>
	/// <returns></returns>
	ParticleData MakeNewParticle();

	/// <summary>
	/// パーティクルの発射管理
	/// </summary>
	void Create();

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix);

	/// <summary>
	/// デバックした値を登録
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// デバックした値を取得
	/// </summary>
	void ApplyDebugParam();
};