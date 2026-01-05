#pragma once
#include"IWeapon.h"

class Sword : public IWeapon, public GameEngine::GameObject {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 所有者の行列を設定
	/// </summary>
	/// <param name="position"></param>
	void SetOwnerPosition(GameEngine::WorldTransform* OwnerTransform) override { worldTransform_.SetParent(OwnerTransform); };

	//　トランスフォームを設定
	void SetTransform(const Transform& transform) override {
		worldTransform_.transform_ = transform;
	}

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransform& GetWorldTransform() override { return worldTransform_; }

	/// <summary>
	/// 当たり判定を取得
	/// </summary>
	/// <returns></returns>
	std::vector<GameEngine::Collider*> GetCollider() override {
		std::vector<GameEngine::Collider*> result;
		for (size_t i = 0; i < colliders_.size(); ++i) {
			result.push_back(colliders_[i].get());
		}
		return result;
	}

	std::vector<std::unique_ptr<GameEngine::SphereCollider>>& GetColliders() { return colliders_; }

	// ダメージ
	int32_t GetDamage() { return damage_; }

private:

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 球の当たり判定
	std::vector<std::unique_ptr<GameEngine::SphereCollider>> colliders_;
	Matrix4x4 colliderWorldMatrix_;
	Vector3 colliderPos_;

	// ダメージ
	int32_t damage_ = 5;
};