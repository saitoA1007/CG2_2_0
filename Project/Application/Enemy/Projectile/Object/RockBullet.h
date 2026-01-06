#pragma once
#include"Application/Enemy/Projectile/IProjectile.h"
#include"Extension/CustomMaterial/IceRockMaterial.h"
#include"Application/Effect/EffectManager.h"

class RockBullet : public IProjectile {
public:

	RockBullet(EffectManager* effectManager);

	void Initialize(const Vector3& pos, const Vector3& dir) override;

	void Update() override;

public:

	bool IsAlive() const override { return isAlive_; }

	GameEngine::WorldTransform& GetWorldTransform() override { return worldTransform_; }

	GameEngine::Collider* GetCollider() override { return collider_.get(); }

	IceRockMaterial* GetMaterial() { return iceMaterial_.get(); }

	ProjectileType GetType() const override { return ProjectileType::Rock; }

private:

	EffectManager* effectManager_ = nullptr;

	bool isAlive_ = true;

	GameEngine::WorldTransform worldTransform_;

	std::unique_ptr<GameEngine::SphereCollider> collider_;

	// マテリアル
	std::unique_ptr<IceRockMaterial> iceMaterial_;

	// 値の保存グループ名
	const std::string groupName_ = "Enemy_RockBullet";

	Vector3 velocity_;

	float timer_ = 0.0f;

	uint32_t breakSH_ = 0;

private: // 調整項目

	// 速度
	float speed_ = 20.0f;

	// 生存時間
	float lifeTime_ = 30.0f;

private:

	/// <summary>
	/// 当たり判定
	/// </summary>
	/// <param name="result"></param>
	void OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result);

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};