#pragma once
#include"WorldTransform.h"
#include"Material.h"
#include"Collider.h"

// 壁の状態
enum class WallState {
	Normal,  // 通常壁
	Strengthen, // 強化壁
	None, // 空
};

class Wall : public GameEngine::GameObject {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="transform"></param>
	/// <param name="respawnTime"></param>
	/// <param name="maxHp"></param>
	void Initialilze(const Transform& transform,float respawnTime, int32_t maxHp);

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
	/// マテリアルを取得
	/// </summary>
	/// <returns></returns>
	GameEngine::Material& GetMaterial() { return material_; }

	/// <summary>
	/// 当たり判定を取得
	/// </summary>
	/// <returns></returns>
	GameEngine::Collider* GetCollider() { return collider_.get(); }

	/// <summary>
	/// 現在の壁の状態を取得
	/// </summary>
	/// <returns></returns>
	WallState GetWallState() const { return wallState_; }

	/// <summary>
	/// OBBのデータを取得
	/// </summary>
	/// <returns></returns>
	OBB GetOBBData();

	/// <summary>
	/// 生存フラグを取得する
	/// </summary>
	/// <returns></returns>
	bool GetIsAlive() const { return isAlive_; }

	// 壁が壊れるパーティクのフラグ
	bool IsBreakParticleActive() { return isBreakParticleActive_; }

	Vector3 GetWorldPosition() { return worldTransform_.GetWorldPosition(); }

private: // 固定値

	// 復活までの時間
	float respawnTime_ = 1.0f;

	// 壁の耐久地
	int32_t maxHp_ = 1;

private:
	// ワールド行列
	GameEngine::WorldTransform worldTransform_;
	// マテリアル
	GameEngine::Material material_;

	// 壁の状態
	WallState wallState_ = WallState::Normal;

	// 現在のタイマー
	float respawnTimer_ = 0.0f;

	// 現在のhp
	int32_t currentHp_ = 1;

	// 生存フラグ
	bool isAlive_ = true;

	bool isBreakParticleActive_ = false;

	// obbの当たり判定
	std::unique_ptr<GameEngine::OBBCollider> collider_;

private:

	/// <summary>
	/// 当たり判定
	/// </summary>
	void OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result);

	/// <summary>
	/// 壁の状態に応じてステータスを変える
	/// </summary>
	void ChangeWallState();
};