#pragma once
#include<array>
#include"WorldTransform.h"
#include"Material.h"
#include"Extension/CustomMaterial/IceRockMaterial.h"
#include"ParticleSystem/ParticleBehavior.h"

#include"BreakEffect.h"

class EnemyDestroyEffect {
public:

	struct ParticleData {
		float timer = 0.0f;
		float maxTime = 1.0f;
		Vector3 scale;
		Vector3 velocity;
		GameEngine::WorldTransform worldTransform;
		std::unique_ptr<IceRockMaterial> material;
	};

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(const uint32_t& texture,const uint32_t& breakTextuer, const Vector3& emitPos);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix);

public:

	/// <summary>
	///　パーティクルデータ
	/// </summary>
	/// <returns></returns>
	std::array<ParticleData, 8>& GetParticleDatas() { return particleData_; }

	std::unique_ptr<GameEngine::ParticleBehavior>& GetSmallParticle() { return smallParticle_; }

	// 行列を取得
	//GameEngine::WorldTransform& GetBreakWorldTransform() { return breakWorldTransform_; }
	/// <summary>
	/// マテリアルを取得
	/// </summary>
	/// <returns></returns>
	GameEngine::Material& GetMaterial() { return material_; }

	bool IsFinished()const { return isFinished_; }

	std::unique_ptr<BreakEffect> breakEffect_;

private:

	std::array<ParticleData, 8> particleData_;

	Vector3 emitPos_;

	bool isFinished_ = false;

	// デバック用
	std::string kGroupName_ = "IceFall";
	Vector4 rimColor;
	Vector4 specularColor;

	std::string name_ = "EnemyDestroyEffect";

	// 細かい粒子
	std::unique_ptr<GameEngine::ParticleBehavior> smallParticle_;

	// サイズ
	float scaleMin_ = 1.0f;
	float scaleMax_ = 1.0f;
	// 速度
	float speedMin_ = 10.0f;
	float speedMax_ = 10.0f;
	// 範囲
	float spawnPosMin_ = 1.0f;
	float spawnPosMax_ = 1.0f;

	float timer_ = 0.0f;
	float maxTime_ = 0.2f;


	// マテリアル
	GameEngine::Material material_;
private:

	/// <summary>
	/// デバックした値を登録
	/// </summary>
	void RegisterBebugParam();

	void ApplyDebugEffectParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam(IceRockMaterial::MaterialData& iceMaterial);
};

