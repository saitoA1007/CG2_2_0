#pragma once
#include "WorldTransform.h"
#include "Collider.h"

enum ProjectileType {
	Rock, // 岩
	Beam, // ビーム

	MaxCount // 数
};

class IProjectile {
public:
	virtual ~IProjectile() = default;

	// 初期化処理
	virtual void Initialize(const Vector3& pos, const Vector3& dir) = 0;

	// 更新処理
	virtual void Update() = 0;

	// 生存フラグ
	virtual bool IsAlive() const = 0;

	// ワールド行列を取得
	virtual GameEngine::WorldTransform& GetWorldTransform() = 0;

	// 当たり判定を取得
	virtual GameEngine::Collider* GetCollider() = 0;

	// 弾のタイプを取得
	virtual ProjectileType GetType() const = 0;
};

// サンプル
//class RockBullet : public IProjectile {
//public:
//
//	void Initialize(const Vector3& pos, const Vector3& dir) override;
//
//	void Update() override;
//
//	bool IsAlive() const override { return isAlive_; }
//
//	void Destroy() override;
//
//	GameEngine::WorldTransform& GetWorldTransform() override { return worldTransform_; }
//
//	GameEngine::Collider* GetCollider() override { return collider_.get(); }
//
//	ProjectileType GetType() const override { return ProjectileType::Rock; }
//
//private:
//
//	bool isAlive_ = false;
//
//	GameEngine::WorldTransform worldTransform_;
//
//	std::unique_ptr<GameEngine::SphereCollider> collider_;
//
//	Vector3 velocity_;
//
//	float speed_ = 20.0f;
//};