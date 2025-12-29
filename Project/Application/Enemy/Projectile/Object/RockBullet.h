#pragma once
#include"Application/Enemy/Projectile/IProjectile.h"

class RockBullet : public IProjectile {
public:

	void Initialize(const Vector3& pos, const Vector3& dir) override{
		pos;
		dir;
	}

	void Update() override{}

	bool IsAlive() const override { return isAlive_; }

	void Destroy() override{}

	GameEngine::WorldTransform& GetWorldTransform() override { return worldTransform_; }

	GameEngine::Collider* GetCollider() override { return collider_.get(); }

	ProjectileType GetType() const override { return ProjectileType::Rock; }

private:

	bool isAlive_ = false;

	GameEngine::WorldTransform worldTransform_;

	std::unique_ptr<GameEngine::SphereCollider> collider_;

	Vector3 velocity_;

	float speed_ = 20.0f;
};