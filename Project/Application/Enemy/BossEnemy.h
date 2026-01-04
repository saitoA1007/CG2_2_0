#pragma once
#include<array>
//#include<functional>

#include"WorldTransform.h"
#include"Collider.h"

#include"BossContext.h"
#include"State/IBossState.h"

class BossEnemy {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(EnemyProjectileManager* projectile);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(const Vector3& targetPos);

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
	uint32_t GetMaxHp() const { return kMaxHp_; }

	// 現在のhp
	uint32_t GetCurrentHp() const { return bossContext_.hp; }

private:

	// 最大hp
	uint32_t kMaxHp_ = 5;

	// 当たり判定の大きさ
	float collisionRadius_ = 2.5f;

private:

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// ボスの共通パラメータ
	BossContext bossContext_;

	// 生存フラグ
	bool isAlive_ = true;

	// 行動状態を保存
	std::array<std::unique_ptr<IBossState>, static_cast<size_t>(BossState::MaxCount)> statesTable_;
	// 現在の状態
	IBossState* currentState_ = nullptr;

	// 球の当たり判定
	std::unique_ptr<GameEngine::SphereCollider> collider_;

	// 現在の状態
	BossState bossState_ = BossState::In;

	// ヒットフラグ
	bool isHit_ = false;

	// ヒット演出で使用
	float alpha_ = 1.0f;
	float hitTimer_ = 0.0f;
	float maxHitTime_ = 1.0f;

private:

	/// <summary>
	/// 当たり判定
	/// </summary>
	void OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result);

	void OnCollisionStay([[maybe_unused]] const GameEngine::CollisionResult& result);

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};