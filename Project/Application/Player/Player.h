#pragma once
#include"WorldTransform.h"
#include"InputCommand.h"
#include"Collider.h"

class Player {
public:

	// プレイヤー情報
	struct playerInfo {
		Vector3 move = {0.0f,0.0f,0.0f}; // 移動処理
		bool isMove = false; // 移動フラグ
	};

public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="model"></param>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="inputCommand"></param>
	void Update(GameEngine::InputCommand* inputCommand);

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

	/// <summary>
	/// プレイヤーの位置を取得
	/// </summary>
	/// <returns></returns>
	Vector3 GetPlayerPos() { return worldTransform_.GetWorldPosition(); }

	/// <summary>
	/// カメラ方向へ向けるための行列を設定
	/// </summary>
	/// <param name="vpMatrix"></param>
	void SetRotateMatrix(const Matrix4x4& rotateMatrix) { rotateMatrix_ = rotateMatrix; }

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

private:

	// ジャンプの高さ
	float kJumpHeight_ = 4.0f;
	// ジャンプする時間
	float kJumpMaxTime_ = 0.65f;

	// 移動速度
	float kMoveSpeed_ = 0.2f;

	// 旋回時間
	float kTurnTime_ = 1.0f;

	// 当たり判定の半径
	float collisionRadius_ = 1.0f;

private:

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 生存フラグ
	bool isAlive_ = true;

	// ジャンプフラグ
	bool isJump_ = false;
	
	// ジャンプタイマー
	float jumpTimer_ = 0.0f;

	// ベクトル変換用の行列
	Matrix4x4 rotateMatrix_;

	// 旋回するために必要な変数
	float turnTimer_ = 0.0f;
	float targetRotateY_ = 0.0f;

	// 球の当たり判定
	std::unique_ptr<GameEngine::SphereCollider> collider_;

private:

	/// <summary>
	/// プレイヤーの入力処理
	/// </summary>
	/// <param name="inputCommand"></param>
	void ProcessMoveInput(GameEngine::InputCommand* inputCommand,playerInfo& playerInfo);

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move(playerInfo& playerInfo);

	/// <summary>
	/// ジャンプする処理
	/// </summary>
	void JumpUpdate();

	/// <summary>
	/// 当たり判定
	/// </summary>
	/// <param name="result"></param>
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