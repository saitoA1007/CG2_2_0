#pragma once
#include"Model.h"
#include"WorldTransform.h"
#include"InputCommand.h"

class Player {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="model"></param>
	void Initialize(GameEngine::Model* model);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="inputCommand"></param>
	void Update(GameEngine::InputCommand* inputCommand);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="VPMatrix"></param>
	void Draw(const Matrix4x4& VPMatrix);

	/// <summary>
	/// プレイヤーの位置を取得
	/// </summary>
	/// <returns></returns>
	Vector3 GetPlayerPos() { return worldTransform_.GetWorldPosition(); }

private:

	// 自キャラモデル
	GameEngine::Model* playerModel_ = nullptr;

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 移動速度
	static inline const float kMoveSpeed_ = 0.2f;

	// ジャンプフラグ
	bool isJump_ = false;
	// ジャンプの高さ
	static inline const float kJumpHeight_ = 4.0f;
	// ジャンプする時間
	static inline const float kJumpTime_ = 0.65f;
	// ジャンプタイマー
	float jumpTimer_ = 0.0f;

private:

	/// <summary>
	/// プレイヤーの入力処理
	/// </summary>
	/// <param name="inputCommand"></param>
	void ProcessMoveInput(GameEngine::InputCommand* inputCommand);

	/// <summary>
	/// ジャンプする処理
	/// </summary>
	void JumpUpdate();
};