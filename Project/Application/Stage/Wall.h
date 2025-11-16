#pragma once
#include"WorldTransform.h"

// 壁の状態
enum class WallState {
	Normal,  // 通常壁
	Strengthen, // 強化壁
	None, // 空
};

class Wall {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="transform"></param>
	/// <param name="respawnTime"></param>
	/// <param name="maxHp"></param>
	void Initialilze(const Transform& transform,float respawnTime, uint32_t maxHp);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

private: // 固定値

	// 復活までの時間
	float respawnTime_ = 0.0f;

	// 壁の耐久地
	uint32_t maxHp_ = 0;

private:
	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 壁の状態
	WallState wallState_ = WallState::Normal;

	// 現在のタイマー
	float respawnTimer_ = 0.0f;

	// 現在のhp
	uint32_t currentHp_ = 0;

	// 生存フラグ
	bool isAlive_ = true;
};