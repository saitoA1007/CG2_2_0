#include"FollowCameraController.h"
#include"MyMath.h"
#include"EasingManager.h"
#include<numbers>
#include"FPSCounter.h"
using namespace GameEngine;

void FollowCameraController::Initialize() {
	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},position_ }, 1280, 720);
}

void FollowCameraController::Update(GameEngine::InputCommand* inputCommand) {

	// 追従する位置を求める
	FollowPosition();

	if (isLockOn_) {
		// プレイヤーからターゲットへの方向ベクトル
		Vector3 toTarget = targetPos_ - basePos_;
		float distance = Length(toTarget);

		if (distance > 0.0f) {
			toTarget *= -1.0f;
			float targetRotateX = std::atan2f(toTarget.x, toTarget.z);

			// 範囲に収める
			float diffX = targetRotateX - rotateMove_.x;
			while (diffX >= std::numbers::pi_v<float>) { diffX -= std::numbers::pi_v<float> *2.0f; }
			while (diffX < -std::numbers::pi_v<float>) { diffX += std::numbers::pi_v<float> *2.0f; }

			// 現在の角度から目標角度へ補間
			rotateMove_.x += diffX * kLockOnRotateSpeed;
		}
	} else {
		// カメラ操作
		if (inputCommand->IsCommandActive("CameraMoveLeft")) {
			rotateMove_.x += 0.04f;
		}

		if (inputCommand->IsCommandActive("CameraMoveRight")) {
			rotateMove_.x -= 0.04f;
		}
	}

	// カメラのロックオンを切り替える
	if (inputCommand->IsCommandActive("CameraLockOn")) {
		isLockOn_ = !isLockOn_;
	}

	// 球面座標系で移動
	position_.x = basePos_.x + kDistance_ * std::sinf(rotateMove_.y) * std::sinf(rotateMove_.x);
	position_.y = basePos_.y + kDistance_ * std::cosf(rotateMove_.y);
	position_.z = basePos_.z + kDistance_ * std::sinf(rotateMove_.y) * std::cosf(rotateMove_.x);

	// 回転行列に変換
	rotateMatrix_ = LookAt(position_, basePos_, { 0.0f,1.0f,0.0f });

	// ワールド行列
	Matrix4x4 worldMatrix_ = rotateMatrix_;
	worldMatrix_.m[3][0] = position_.x;
	worldMatrix_.m[3][1] = position_.y;
	worldMatrix_.m[3][2] = position_.z;

	// ワールド行列を設定
	camera_->SetWorldMatrix(worldMatrix_);
	// ワールド行列から更新する
	camera_->UpdateFromWorldMatrix();
}

void FollowCameraController::FollowPosition() {
	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	TargetCoordinate_ = playerPos_ + playerVelocity_ * (kVelocityBias * FpsCounter::deltaTime);

	// 座標補間によりゆったり追従
	basePos_ = Lerp(basePos_, TargetCoordinate_, kInterpolationRate);
}

Matrix4x4 FollowCameraController::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
	Vector3 f = Normalize(center - eye); // 前方向ベクトル
	Vector3 s = Normalize(Cross(up, f)); // 右方向ベクトル
	Vector3 u = Cross(f, s); // 上方向ベクトル

	Matrix4x4 result = { {
		{ s.x,  s.y, s.z, 0 },
		{ u.x,  u.y, u.z, 0 },
		{ f.x,  f.y, f.z, 0 },
		{ 0.0f, 0.0f, 0.0f, 1}
	} };
	return result;
}