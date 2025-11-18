#pragma once
#include <optional>
#include<array>
#include<functional>

#include"WorldTransform.h"
#include"InputCommand.h"
#include"Collider.h"

// 武器
#include"Application/Weapon/IWeapon.h"

class Player {
public:

	// プレイヤーの行動パターン
	enum class Behavior {
		Normal, // 通常状態
		Attack, // 攻撃状態
		Jump,   // ジャンプ状態
		Dush,   // ダッシュ状態

		MaxCount // 状態の数
	};

public:
	Player() = default;
	~Player() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="model"></param>
	void Initialize(GameEngine::InputCommand* inputCommand);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="inputCommand"></param>
	void Update();

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
	/// プレイヤーの速度を取得
	/// </summary>
	/// <returns></returns>
	Vector3 GetVelocity() const { return velocity_; }

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

	// 移動速度
	float kMoveSpeed_ = 0.2f;

	// 旋回時間
	float kTurnTime_ = 1.0f;

	// ジャンプの高さ
	float kJumpHeight_ = 4.0f;
	// ジャンプする時間
	float kJumpMaxTime_ = 0.65f;

	// ダッシュする速さ
	float kDushSpeed_ = 10.0f;
	// ダッシュする時間
	float kDushMaxTime_ = 2.0f;

	// 当たり判定の半径
	float kCollisionRadius_ = 1.0f;

private:

	// 入力処理
	GameEngine::InputCommand* inputCommand_;

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 速度
	Vector3 velocity_ = {};

	// 生存フラグ
	bool isAlive_ = true;

	// 武器を取得
	IWeapon* weapon_ = nullptr;

	// 球の当たり判定
	std::unique_ptr<GameEngine::SphereCollider> collider_;

	// プレイヤーの振る舞い
	Behavior behavior_ = Behavior::Normal;
	// 振る舞いの変更を管理
	std::optional<Behavior> behaviorRequest_ = std::nullopt;
	// プレイヤーの状態テーブル
	std::array<std::function<void()>, static_cast<size_t>(Behavior::MaxCount)> behaviorsTable_;
	// プレイヤーが指定した状態を行うためのリセット処理
	std::array<std::function<void()>, static_cast<size_t>(Behavior::MaxCount)> resetBehaviorParamTable_;

	// 移動で使用する変数 ============================

	// 移動処理
	Vector3 move = { 0.0f,0.0f,0.0f };
	// 移動フラグ
	bool isMove = false;

	// 旋回処理で使用する変数 =============================

	float turnTimer_ = 0.0f;
	float targetRotateY_ = 0.0f;
	// ベクトル変換用の行列
	Matrix4x4 rotateMatrix_;

	// ジャンプで使用する変数 ===========================

	// ジャンプタイマー
	float jumpTimer_ = 0.0f;

	// ダッシュで使用する変数 ==================================
	float dushTimer_ = 0.0f;
	// ダッシュする方向を設定する
	Vector3 dushDirection_ = { 0.0f,0.0f,0.0f };

private: // プレイヤーの行動関数

	/// <summary>
	/// プレイヤーの入力処理
	/// </summary>
	/// <param name="inputCommand"></param>
	void ProcessMoveInput();

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move();

	/// <summary>
	/// 通常の更新処理
	/// </summary>
	void NormalUpdate();

	/// <summary>
	/// ジャンプする処理
	/// </summary>
	void JumpUpdate();

	/// <summary>
	/// 攻撃する処理
	/// </summary>
	void AttackUpdate();

	/// <summary>
	/// ダッシュの更新処理
	/// </summary>
	void DushUpdate();

private: // プレイヤーの他の関数

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