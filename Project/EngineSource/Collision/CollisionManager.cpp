#include"CollisionManager.h"
#include"CollisionVisitor.h"
using namespace GameEngine;

void CollisionManager::CheckAllCollisions() {

	// 何も無ければ早期リターン
	if (colliders_.empty()) {
		return;
	}

	// リスト内のペアを総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {
		// コライダーAを取得
		Collider* colliderA = *itrA;

		// イテレーターBはイテレータAの次の要素から回す(重複判定を回避)
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for (; itrB != colliders_.end(); ++itrB) {
			// コライダーBを取得
			Collider* colliderB = *itrB;

			// ペアの当たり判定
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void  CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {

	// 衝突フィルタリング
	if (!IsActiveCollision(colliderA, colliderB)) {
		return;
	}

	// 形状を取得
	CollisionType typeA = colliderA->GetCollisionType();
	CollisionType typeB = colliderB->GetCollisionType();

	// 各形状に応じた当たり判定を取得
	CollisionResult result = CheckCollisionType(typeA, typeB);

	if (result.isHit) {
		// コライダーの衝突時コールバックを呼び出す
		colliderA->OnCollision(result);
		// 反対側の法線情報を渡す
		CollisionResult resultB = result;
		resultB.contactNormal = result.contactNormal * -1.0f;
		colliderB->OnCollision(resultB);
	}
}

bool CollisionManager::IsActiveCollision(Collider* a, Collider* b) {
	return (a->GetCollisionAttribute() & b->GetCollisionMask()) == 0 || (b->GetCollisionAttribute() & a->GetCollisionMask()) == 0;
}

CollisionResult CollisionManager::CheckCollisionType(const CollisionType& typeA, const CollisionType& typeB) {
	// 各形状の当たり判定を取得する
	return std::visit(CollisionVisitor{}, typeA.type, typeB.type);
}