#pragma once
#include"WorldTransform.h"
#include"Collider.h"

class Wall {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const Transform& transform);

	void Update();

public:

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

	/// <summary>
	/// 当たり判定を取得
	/// </summary>
	/// <returns></returns>
	GameEngine::Collider* GetCollider() { return collider_.get(); }

	// AABBのデータを取得
	AABB GetAABBData();

private:
	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// aabbの当たり判定
	std::unique_ptr<GameEngine::AABBCollider> collider_;

	//std::string kGroupName_ = "Wall";

private:

	/// <summary>
	/// 当たり判定
	/// </summary>
	void OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result);
};