#pragma once
#include<list>

#include"Collider.h"

class CollisionManager {
public:

	/// <summary>
	/// 全ての当たり判定を行う
	/// </summary>
	void CheckAllCollisions();

	/// <summary>
	/// コライダーリストをクリアする
	/// </summary>
	void ClearList() { colliders_.clear(); }

	/// <summary>
	/// コライダーをリストに追加する
	/// </summary>
	/// <param name="collider"></param>
	void AddCollider(Collider* collider) { colliders_.push_back(collider); }

private:
	// コライダーリスト
	std::list<Collider*> colliders_;

private:

	/// <summary>
	/// コライダー2つの衝突判定と応答
	/// </summary>
	/// <param name="colliderA">コライダーA</param>
	/// <param name="colliderB">コライダーB</param>
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

	/// <summary>
	/// 衝突出来るか判定する
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	bool IsActiveCollision(Collider* a, Collider* b);

	/// <summary>
	/// 形状に応じた当たり判定を実行する
	/// </summary>
	/// <param name="typeA"></param>
	/// <param name="typeB"></param>
	/// <returns></returns>
	bool CheckCollisionType(const CollisionType& typeA, const CollisionType& typeB);
};