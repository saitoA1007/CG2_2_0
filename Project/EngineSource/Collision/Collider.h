#pragma once
#include"Vector3.h"
#include<cstdint>
#include"Geometry.h"
#include<variant>

// 当たり判定の形状
struct CollisionType {
	// 当たり判定の属性
	std::variant<Sphere, AABB, Segment> type;

	// 形状を取得する
	template<typename T>
	const T* Get() const {return std::get_if<T>(&type);}
	template<typename T>
	T* Get() {return std::get_if<T>(&type);}
};

/// <summary>
/// 当たり判定が持つ基盤の要素
/// </summary>
class Collider {
public:

	// ワールド座標を取得
	virtual Vector3 GetWorldPosition() const = 0;

	// 衝突形状を取得
	virtual CollisionType GetCollisionType() const = 0;

	// 衝突時に呼ばれる関数
	virtual void OnCollision() = 0;

	// 衝突属性を取得
	uint32_t GetCollisionAttribute() const { return collisionAttribute_; }
	// 衝突属性を設定
	void SetCollisionAttribute(const uint32_t& collisionAttribute) { collisionAttribute_ = collisionAttribute; }

	// 衝突マスク
	uint32_t GetCollisionMask() const { return collisionMask_; }
	// 衝突マスク
	void SetCollisionMask(const uint32_t& collisionMask) { collisionMask_ = collisionMask; }

	// 有効状態
	bool IsActive() const { return isActive_; }
	void SetActive(const bool& isActive) { isActive_ = isActive; }

private:
	// 衝突属性(自分)
	uint32_t collisionAttribute_ = 0xffffffff;
	// 衝突マスク(相手)
	uint32_t collisionMask_ = 0xffffffff;
	// 当たり判定の有効化
	bool isActive_ = true;
};

/// <summary>
/// 球の当たり判定
/// </summary>
class SphereCollider : public Collider {
public:

	// 球の当たり判定を登録
	CollisionType GetCollisionType() const override {
		CollisionType collisiontype;
		collisiontype.type = Sphere{ GetWorldPosition(),radius_ };
		return collisiontype;
	}

	const float GetRadius() const { return radius_; }
	void SetRadius(const float& radius) { radius_ = radius; }

private:
	// 半径
	float radius_;
};

/// <summary>
/// AABBの当たり判定
/// </summary>
class AABBCollider : public Collider {
public:

	// AABBの当たり判定を登録する
	CollisionType GetCollisionType() const override {
		CollisionType collisiontype;
		Vector3 pos = GetWorldPosition();
		Vector3 halfSize = size_ * 0.5f;
		collisiontype.type = AABB{ pos - halfSize,pos + halfSize };
		return collisiontype;
	}

	// サイズを取得する
	const Vector3 GetSize() const { return size_; }
	void SetSize(const Vector3& size) { size_ = size; }

private:
	// サイズ
	Vector3 size_;
};


/// <summary>
/// 線分の当たり判定
/// </summary>
class SegmentCollider : public Collider {
public:
	// 線分の当たり判定を登録する
	CollisionType GetCollisionType() const override {
		CollisionType collisiontype;
		Vector3 pos = GetWorldPosition();
		collisiontype.type = Segment{ pos,diff_};
		return collisiontype;
	}

	// 線の方向
	const Vector3 GetDiff() const { return diff_; }
	void SetSize(const Vector3& diff) { diff_ = diff; }

private:
	Vector3 diff_;
};