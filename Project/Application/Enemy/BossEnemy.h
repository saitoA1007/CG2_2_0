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

	/// <summary>
	/// 球のデータを取得
	/// </summary>
	/// <returns></returns>
	Sphere GetSphereData();

	/// <summary>
	/// 生存フラグを取得する
	/// </summary>
	/// <returns></returns>
	bool GetIsAlive() const { return isAlive_; }

	/// <summary>
	/// 透明度を取得
	/// </summary>
	/// <returns></returns>
	float GetAlpha() const { return alpha_; }

private:

	// 最大hp
	uint32_t maxHp_ = 3;

	// 当たり判定の大きさ
	float collisionRadius_ = 2.5f;

private:

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// hp
	uint32_t hp_ = 3;

	// 生存フラグ
	bool isAlive_ = true;

	// 球の当たり判定
	std::unique_ptr<GameEngine::SphereCollider> collider_;

	float alpha_ = 1.0f;

	bool isHit_ = false;

	float hitTimer_ = 0.0f;

	float maxHitTime_ = 1.0f;

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