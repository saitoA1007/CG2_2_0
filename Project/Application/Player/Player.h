#pragma once
#include"WorldTransform.h"
#include"InputCommand.h"
#include"Collider.h"
#include"Camera.h"

class Player {
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="model"></param>
	void Initialize();

	/// <summary>
	/// 更新処理 (カメラ基準移動)
	/// </summary>
	/// <param name="inputCommand"></param>
	/// <param name="camera">メインカメラ</param>
	void Update(GameEngine::InputCommand* inputCommand, const GameEngine::Camera& camera);

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
	/// コライダー取得
	/// </summary>
	GameEngine::Collider* GetCollider() { return collider_.get(); }

    /// <summary>
    /// プレイヤーの当たり判定用の球データを取得
	/// </summary>
    Sphere GetSphereData() const { return sphereData_; }

	// 追加: 状態取得
	bool IsCharging() const { return isCharging_; }
	bool IsAttackDown() const { return isAttackDown_; }

private:
	//==================================================
    // 定数・設定値
    //==================================================

	const std::vector<std::string> kGroupNames = {
		"Player",
		"Player/Charge",
		"Player/WallBounce",
		"Player/GreatWallBounce",
		"Player/Attack",
	};

	// ジャンプの高さ
	float kJumpHeight_ = 4.0f;
	// ジャンプする時間
	float kJumpMaxTime_ = 0.65f;

    // 地上移動速度（秒速）
	float kMoveSpeed_ = 5.0f;
    // 空中移動速度（秒速）
    float kAirMoveSpeed_ = 5.0f;
    // 地上移動加減速量（秒速）
    float kGroundAcceleration_ = 0.5f;
    // 空中移動加減速量（秒速）
    float kAirDeceleration_ = 0.1f;
    // 落下速度の上限（秒速）
	float MaxFallSpeed_ = 2.0f;
    // 落下加減速量（秒速）
    float kFallAcceleration_ = -9.6f;
    // 空中での方向転換のしやすさ
    float AirAcceleration_ = 50.0f;

	//--------- 突撃の設定 ---------//

	// 突撃予備動作時間
    float kPreChargeTime_ = 0.5f;
	// 突撃速度
    float kChargeSpeed_ = 25.0f;

	//--------- 通常壁への跳ね返り設定 ---------//

	// 跳ね上がり後の高さ(上方向速度のイメージ)
    float kWallBounceUpSpeed_ = 10.0f;
	// 跳ね上がり後の壁から離れる距離(前方向速度のイメージ)
    float kWallBounceAwaySpeed_ = 5.0f;
	// 跳ね返り直後の硬直時間
    float kWallBounceLockTime_ = 0.8f;

	//--------- 強化壁への跳ね返り設定 ---------//

	// 跳ね上がり後の高さ
	float kGreatWallBounceUpSpeed_ = 15.0f;
	// 跳ね上がり後の壁から離れる距離
	float kGreatWallBounceAwaySpeed_ = 5.0f;
	// 跳ね返り直後の硬直時間
	float kGreatWallBounceLockTime_ = 1.2f;

	//--------- Attack（空中急降下）の設定 ---------//

    // 落下前の硬直時間
    float kAttackPreDownTime_ = 0.3f;
    // 落下攻撃の落下速度
    float kAttackDownSpeed_ = 10.0f;

private:
	//==================================================
    // メンバ変数
    //==================================================

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

    // プレイヤーの当たり判定用の球データ
    Sphere sphereData_ = { {0.0f,0.0f,0.0f}, 1.f };

    // 速度ベクトル
	Vector3 velocity_ = {0.0f,0.0f,0.0f};
    // XZの目標速度
    Vector3 desiredVelXZ_ = { 0.0f, 0.0f, 0.0f };
    // 最後に移動していたXZ方向（正規化）
    Vector3 lastMoveDir_ = { 0.0f, 0.0f, 1.0f };

	// カメラ基準の前/右(XZ平面)
	Vector3 cameraForwardXZ_ = {0.0f,0.0f,1.0f};
	Vector3 cameraRightXZ_   = {1.0f,0.0f,0.0f};

	// ジャンプフラグ
	bool isJump_ = false;
	// ジャンプタイマー
	float jumpTimer_ = 0.0f;

	// 突進関連フラグ
	bool isPreCharging_ = false;
	bool isCharging_ = false;
	float chargeTimer_ = 0.0f;
	float chargeActiveTimer_ = 0.0f;
	Vector3 chargeDirection_ = { 0.0f,0.0f,1.0f };

	// 壁跳ね返り（硬直）関連
	bool isBounceLock_ = false;
	float bounceLockTimer_ = 0.0f;
	float currentBounceUpSpeed_ = 0.0f;
	float currentBounceAwaySpeed_ = 0.0f;
	float currentBounceLockTime_ = 0.0f;
	Vector3 bounceAwayDir_ = {0.0f, 0.0f, 0.0f};

	// 急降下攻撃フラグ
    bool isAttackDown_ = false;

	// プレイヤー用コライダー（球）
	std::unique_ptr<GameEngine::SphereCollider> collider_;

private:
	void ProcessMoveInput(GameEngine::InputCommand* inputCommand);
	void StartCharge(const Vector3& direction);
	void ChargeUpdate();
	void BounceUpdate();
	void ChargeWallBounce(const Vector3 &bounceDirection, bool isGreatWall);
	void OnCollision(const GameEngine::CollisionResult& result);
	void RegisterBebugParam();
	void ApplyDebugParam();

	// カメラ基準ベクトル更新
	void UpdateCameraBasis(const GameEngine::Camera* camera);
};