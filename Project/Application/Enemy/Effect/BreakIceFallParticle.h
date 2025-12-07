#pragma once
#include<array>
#include"WorldTransform.h"
#include"Extension/CustomMaterial/IceRockMaterial.h"

class BreakIceFallParticle {
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
	void Initialize(const uint32_t& texture,const Vector3& emitPos);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

public:

	/// <summary>
	///　パーティクルデータ
	/// </summary>
	/// <returns></returns>
	std::array<ParticleData, 5>& GetParticleDatas() { return particleData_; }

	bool IsFinished()const { return isFinished_; }

private:
	
	std::array<ParticleData,5> particleData_;

	Vector3 emitPos_;

	bool isFinished_ = false;

	// デバック用
	std::string kGroupName_ = "IceFall";
	Vector4 rimColor;
	Vector4 specularColor;

private:

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam(IceRockMaterial::MaterialData& iceMaterial);
};

