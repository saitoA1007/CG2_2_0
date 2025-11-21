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
	/// <param name="inputCommand"></param>
	/// <param name="targetPos"></param>
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
	void SetFollowPos(const Vector3& targetPos, const Vector3& velocity) {
		targetPos_ = targetPos;
		targetPos_.y = 1.0f;
		targetVelocity_ = velocity;
	}

private:

	// カメラ
	std::unique_ptr<GameEngine::Camera> camera_;
	Vector3 position_ = { 0.0f,4.0f,-10.0f };
	Vector3 target = {};

	// 距離
	static inline const float kDistance_ = 40.0f;

	// 回転の移動量
	Vector2 rotateMove_ = { 3.1f,1.0f };

	// 回転行列
	Matrix4x4 rotateMatrix_;

	// 追従対称の残像処理
	Vector3 interTarget_ = {};
	Vector3 targetPos_ = {};
	Vector3 targetVelocity_ = {};

	// カメラの目標座標
	Vector3 TargetCoordinate_ = {};

	// 座標補間割合
	float kInterpolationRate = 0.2f;
	// 速度掛け率
	float kVelocityBias = -0.5f;

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