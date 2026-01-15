#pragma once
#include"Camera.h"
#include"InputCommand.h"

class FollowCameraController {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="inputCommand">入力コマンド</param>
	/// <param name="targetPos">目標位置</param>
	void Update(GameEngine::InputCommand* inputCommand);

	/// <summary>
	/// カメラデータ
	/// </summary>
	/// <returns></returns>
	GameEngine::Camera& GetCamera() const { return *camera_.get(); }

	/// <summary>
	/// vpMatrixを取得
	/// </summary>
	/// <returns></returns>
	Matrix4x4 GetRotateMatrix() const { return rotateMatrix_; }

	/// <summary>
	/// 追従する位置と速度を設定する
	/// </summary>
	/// <param name="targetPos"></param>
	/// <param name="velocity"></param>
	void SetFollowPos(const Vector3& playerPos, const Vector3& velocity) {
		playerPos_ = playerPos;
		playerPos_.y = 1.0f;
		playerVelocity_ = velocity;
	}

	// カメラを向ける位置を設定
	void SetLockOnPos(const Vector3& targetPos) {
		targetPos_ = targetPos;
	}

	// ロックオンフラグを取得
	bool GetIsLockOn() const { return isLockOn_; }

	// ロックオンフラグを設定
	void SetIsChangeActive(const bool& isActive) { isChangeActive_ = !isActive; }
		
private:

	// カメラ
	std::unique_ptr<GameEngine::Camera> camera_;
	Vector3 position_ = { 0.0f,4.0f,-10.0f };
	Vector3 basePos_ = {};

	// 距離
	static inline const float kDistance_ = 60.0f;

	// 回転の移動量
	Vector2 rotateMove_ = { 3.1f,1.0f };

	// 回転行列
	Matrix4x4 rotateMatrix_;

	// 追従対称の残像処理
	Vector3 interTarget_ = {};
	Vector3 playerPos_ = {};
	Vector3 playerVelocity_ = {};

	// カメラの目標座標
	Vector3 TargetCoordinate_ = {};

	// 座標補間割合
	float kInterpolationRate = 0.2f;
	// 速度掛け率
	float kVelocityBias = -0.5f;

	// カメラを向ける位置
	Vector3 targetPos_ = {};

	// ロックオン時の回転速度
	float kLockOnRotateSpeed = 0.1f;

	bool isLockOn_ = false;

	// カメラの切り替えを可能か判断
	bool isChangeActive_ = false;

	// 切り替え音
	uint32_t changeSH_ = 0;

private:

	/// <summary>
	/// カメラをターゲットの方向に向かせる
	/// </summary>
	/// <param name="eye">カメラの位置</param>
	/// <param name="center">ターゲットの位置</param>
	/// <param name="up">向き</param>
	/// <returns></returns>
	Matrix4x4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);

	/// <summary>
	/// カメラの追尾位置に補間
	/// </summary>
	void FollowPosition();
};