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