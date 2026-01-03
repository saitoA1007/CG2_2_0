#pragma once
#include <optional>
#include<array>
#include<functional>

#include"WorldTransform.h"
#include"InputCommand.h"
#include"Collider.h"

// 武器
//#include"Application/Weapon/IWeapon.h"
#include"Application/Weapon/Sword.h"

class Player : public GameEngine::GameObject {
public:

	// プレイヤーの行動パターン
	enum class Behavior {
		Normal, // 通常状態
		Attack, // 攻撃状態
		Jump,   // ジャンプ状態
		Dush,   // ダッシュ状態

		MaxCount // 状態の数
	};

	// 攻撃用定数
	struct ConstAttack {
		float maxTime = 1.0f;
		float radius = 0.0f;
	};

	// 攻撃用ワーク
	struct WorkAttack {
		int32_t comboIndex = 0; // 現在のコンボ段階
		int32_t inComboPhase = 0; // 1コンボの中でどのフェーズか
		bool isComboNext = false; // コンボが次の段階に進むフラグ
		float maxTime_ = 0.0f; // 1コンボの最大時間
		float timer_ = 0.0f; // 通常行動に戻るまでの時間
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

	/// <summary>
	/// 武器を設定
	/// </summary>
	/// <param name="weapon"></param>
	void SetWeapon(Sword* weapon) { weapon_ = weapon; }

	/// <summary>
	/// 保持している武器を取得
	/// </summary>
	/// <returns></returns>
	Sword* GetWeapon() { return weapon_; }

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
	/// カメラ情報を取得
	/// </summary>
	/// <param name="rotateMatrix"></param>
	/// <param name="isCameraLockOn"></param>
	void SetCameraInfo(const Matrix4x4& rotateMatrix, const bool& isCameraLockOn,const Vector3& target) {
		rotateMatrix_ = rotateMatrix;
		isCameraLockOn_ = isCameraLockOn;
		targetPos_ = target;
	}

	/// <summary>
	/// 球のデータを取得
	/// </summary>
	/// <returns></returns>
	Sphere GetSphereData();

	/// <summary>
	/// プレイヤーの現在の振る舞い
	/// </summary>
	/// <returns></returns>
	Behavior GetPlayerBehavior() const { return behavior_; }

	/// <summary>
	/// ヒットフラグ
	/// </summary>
	/// <returns></returns>
	bool IsHit()const { return isHit_; }

	/// <summary>
	/// 攻撃フラグ
	/// </summary>
	/// <returns></returns>
	bool IsAttack() const { return isAttack_; }

private: // 調整項目

	// 移動速度
	float kMoveSpeed_ = 0.2f;

	// 旋回時間
	float kTurnTime_ = 0.2f;

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

	std::vector<std::string> kGroupNames_ = {
		"Player_Combo",
	};

	// 入力処理
	GameEngine::InputCommand* inputCommand_;

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 速度
	Vector3 velocity_ = {};

	// 生存フラグ
	bool isAlive_ = true;

	// 武器を取得
	Sword* weapon_ = nullptr;

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

	// ヒットフラグ
	bool isHit_ = false;

	// 攻撃フラグ
	bool isAttack_ = false;

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
	bool isCameraLockOn_ = false;
	Vector3 targetPos_ = {};

	// ジャンプで使用する変数 ===========================

	// ジャンプタイマー
	float jumpTimer_ = 0.0f;

	// ダッシュで使用する変数 ==================================
	float dushTimer_ = 0.0f;
	// ダッシュする方向を設定する
	Vector3 dushDirection_ = { 0.0f,0.0f,0.0f };

	// 攻撃で使用する変数 ===========================================

	// コンボ回数
	static inline const int kComboNum = 3;

	// コンボ定数表
	std::array<ConstAttack, kComboNum> kConstAttacks_;

	// 攻撃ワーク
	WorkAttack workAttack_;

	// 武器の位置
	Transform weaponTransform_ = {};

	float theta_ = 0.0f;

	// ダメージを受けた時の挙動に使用する変数 =============================

	// 当たった方向
	Vector3 hitDirection_ = {};

	// ノックバックするスピード
	float knockbackSpeed_ = 0.0f;

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