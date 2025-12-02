#pragma once
#include"WorldTransform.h"
#include"Collider.h"

class BossEnemy {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
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

	// 球のデータを取得
	Sphere GetSphereData();

	// 生存フラグを取得する
	bool GetIsAlive() const { return isAlive_; }

	// 透明度を取得
	float GetAlpha() const { return alpha_; }

	// ワールド座標を取得する
	Vector3 GetPosition() const { return worldTransform_.GetWorldPosition(); }

	// 最大hpを取得する
	uint32_t GetMaxHp() const { return maxHp_; }

	// 現在のhp
	uint32_t GetCurrentHp() const { return hp_; }

private:

	// 最大hp
	uint32_t maxHp_ = 2;

	// 当たり判定の大きさ
	float collisionRadius_ = 2.5f;

private:

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// hp
	uint32_t hp_ = 5;

	// 生存フラグ
	bool isAlive_ = true;

	// 球の当たり判定
	std::unique_ptr<GameEngine::SphereCollider> collider_;

	float alpha_ = 1.0f;

	bool isHit_ = false;

	float hitTimer_ = 0.0f;

	float maxHitTime_ = 1.0f;

	// 角度
	float theta_ = 1.5f;

private:

	/// <summary>
	/// 当たり判定
	/// </summary>
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