#include"CameraController.h"
#include"MyMath.h"
#include <algorithm>
#include <cmath>
#include <random>
#include "EasingManager.h"
#include "FPSCounter.h"
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
using namespace GameEngine;

void CameraController::Initialize() {
	camera_ = std::make_unique<Camera>();
	camera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},position_ }, 1280, 720);
	// 現在値を目標値に同期
	desiredTargetPos_ = targetPos_;
    desiredTargetLookAt_ = targetLookAt_;
	desiredTargetRotate_ = targetRotate_;
	desiredTargetFov_ = targetFov_;
	prevTargetPos_ = targetPos_; // 初期化
}

void CameraController::Update(GameEngine::InputCommand* inputCommand, GameEngine::Input* rawInput) {
	// ターゲット種類に応じて目標値更新
	std::visit([this](auto&& value) {
		using T = std::decay_t<decltype(value)>;
		if constexpr (std::is_same_v<T, Vector3>) {
			UpdateTargetVector3(value);
		} else if constexpr (std::is_same_v<T, Line>) {
			UpdateTargetLine(value);
		} else if constexpr (std::is_same_v<T, std::vector<Vector3>>) {
			UpdateTargetVector3Array(value);
		}
	}, target_);

	// スムーズ補間係数
	const float t = 0.1f;

	// 座標系種類でカメラ位置追従 (補間前に回転入力反映)
	switch (cameraCoordinateType_) {
	case CameraCoodinateType::Cartesian:  UpdateCartesian(inputCommand);  break;
	case CameraCoodinateType::Spherical: UpdateSpherical(inputCommand, rawInput); break;
	}
	// 位置・回転(Euler)・FOV補間
	targetPos_ = Lerp(targetPos_, desiredTargetPos_, t);
    targetLookAt_ = Lerp(targetLookAt_, desiredTargetLookAt_, t);
	targetRotate_ = Lerp(targetRotate_, desiredTargetRotate_, t);
	targetFov_ = Lerp(targetFov_, desiredTargetFov_, t);

    // カメラシェイク適用
	Vector3 eye = targetLookAt_;
    Vector3 center = targetPos_;
	// ターゲット移動による自動回転(スpherical時のみ有効)
	ApplyAutoRotate(eye, center);

	// シェイク
	if (cameraShakePower_ > 0.0f && cameraShakeElapsedTime_ < cameraShakeMaxTime_) {
		cameraShakeElapsedTime_ += FpsCounter::deltaTime;
		float tn = std::clamp(cameraShakeElapsedTime_ / std::max(cameraShakeMaxTime_, 0.0001f), 0.0f, 1.0f);
		float amp = EaseOutQuad(cameraShakePower_, 0.0f, tn);
		float currentDistance = Length(center - eye);
		float distanceScale = std::clamp(kDistance_ / std::max(currentDistance, 0.0001f), 0.1f, 1.0f);
		amp *= distanceScale;
		static thread_local std::mt19937 rng{ std::random_device{}() };
		std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
		Vector3 noise{ dist(rng), dist(rng) * 0.5f, dist(rng) };
		if (!enableShakeX_) noise.x = 0.0f; if (!enableShakeY_) noise.y = 0.0f; if (!enableShakeZ_) noise.z = 0.0f;
		switch (shakeOrigin_) {
		case ShakeOrigin::CameraPosition: eye.x += noise.x * amp; eye.y += noise.y * amp; eye.z += noise.z * amp; break;
		case ShakeOrigin::TargetPosition: center.x += noise.x * amp; center.y += noise.y * amp; center.z += noise.z * amp; break;
		case ShakeOrigin::TargetAndCameraPosition:
			eye.x += noise.x * amp * 0.5f; eye.y += noise.y * amp * 0.5f; eye.z += noise.z * amp * 0.5f;
			center.x += noise.x * amp * 0.5f; center.y += noise.y * amp * 0.5f; center.z += noise.z * amp * 0.5f; break;
		}
	}

	Matrix4x4 rotateMatrix = LookAt(eye, center, { 0.0f,1.0f,0.0f });
	Matrix4x4 worldMatrix = rotateMatrix;
	worldMatrix.m[3][0] = eye.x; worldMatrix.m[3][1] = eye.y; worldMatrix.m[3][2] = eye.z;
	camera_->SetWorldMatrix(worldMatrix);
	camera_->SetProjectionMatrix(targetFov_, 1280, 720, 0.1f, 1000.0f);
	camera_->UpdateFromWorldMatrix();
}

void CameraController::ApplyAutoRotate(const Vector3& eye, const Vector3& center) {
	if (cameraCoordinateType_ != CameraCoodinateType::Spherical) { prevTargetPos_ = center; return; }
	// 前フレームからのターゲット移動量
	Vector3 delta = center - prevTargetPos_;
	prevTargetPos_ = center;
	// カメラの現在の視線ベクトル
	Vector3 forward = Normalize(center - eye);
	// カメラ右方向
	Vector3 up{0.0f,1.0f,0.0f};
	Vector3 right = Normalize(Cross(up, forward));
	// カメラ平面でのターゲット横移動量(右方向成分)
	float lateral = Dot(delta, right); // 右に動けば +, 左なら -
	// カメラ平面でのターゲット前進量(前方向成分)
	float toward = Dot(delta, forward); // 手前(eyeに近づく)は負になるため符号反転で倍率に使用
	float approachFactor = std::max(-toward, 0.0f) * autoRotateApproachScale_; // 接近時のみ加算
	// 後方移動の特別扱い: forward反対方向(=後ろ)に十分動いている場合は右回転を強制
	/*const float backThreshold = 0.001f;
	if (toward < -backThreshold && std::fabs(lateral) < backThreshold) {
		rotateMove_.x += autoRotateYawSensitivity_ * autoRotateOverallGain_ * (1.0f + std::max(-toward, 0.0f));
		return;
	}*/
	// 角度更新 (横移動 × 感度 × 接近倍率補正)
	rotateMove_.x += lateral * autoRotateYawSensitivity_ * autoRotateOverallGain_ * (1.0f + approachFactor);
	// wrap角度
	if (rotateMove_.x > 6.283185f) rotateMove_.x -= 6.283185f; else if (rotateMove_.x < -6.283185f) rotateMove_.x += 6.283185f;
}

void CameraController::UpdateTargetVector3(const Vector3& v) {
	desiredTargetPos_ = v;
	Vector3 dir = Normalize(v - position_);
	float yaw = std::atan2(dir.x, dir.z);
	float pitch = std::asin(std::clamp(dir.y, -1.0f, 1.0f));
	desiredTargetRotate_ = { pitch, yaw, 0.0f };
}

void CameraController::UpdateTargetLine(const Line& line) {
	Vector3 dir = Normalize(line.diff);
	float lookDistance = 5.0f;
	desiredTargetPos_ = line.origin + dir * lookDistance;
	float yaw = std::atan2(dir.x, dir.z);
	float pitch = std::asin(std::clamp(dir.y, -1.0f, 1.0f));
	desiredTargetRotate_ = { pitch, yaw, 0.0f };
	float length = std::sqrt(line.diff.x*line.diff.x + line.diff.y*line.diff.y + line.diff.z*line.diff.z);
	desiredTargetFov_ = std::clamp(0.45f + length * 0.5f, 0.3f, 0.9f);
}

void CameraController::UpdateTargetVector3Array(const std::vector<Vector3>& arr) {
	if (arr.empty()) { UpdateTargetVector3({0.0f,0.0f,0.0f}); return; }
	Vector3 sum{0.0f,0.0f,0.0f}; for (const auto& p : arr) { sum.x += p.x; sum.y += p.y; sum.z += p.z; }
	Vector3 avg{ sum.x / (float)arr.size(), sum.y / (float)arr.size(), sum.z / (float)arr.size() };
	desiredTargetPos_ = avg;
	Vector3 dir = Normalize(avg - position_);
	float yaw = std::atan2(dir.x, dir.z);
	float pitch = std::asin(std::clamp(dir.y, -1.0f, 1.0f));
	desiredTargetRotate_ = { pitch, yaw, 0.0f };
	float maxDistSq = 0.0f;
	for (const auto& p : arr) { float dx = p.x - avg.x; float dy = p.y - avg.y; float dz = p.z - avg.z; float d2 = dx*dx + dy*dy + dz*dz; if (d2 > maxDistSq) maxDistSq = d2; }
	float radius = std::sqrt(maxDistSq);
	desiredTargetFov_ = std::clamp(0.45f + radius * 0.8f, 0.3f, 0.9f);
}

void CameraController::UpdateCartesian(GameEngine::InputCommand* inputCommand) {
	if (inputCommand && inputCommand->IsCommandAcitve("CameraMoveLeft")) { position_.x -= 0.5f; }
	if (inputCommand && inputCommand->IsCommandAcitve("CameraMoveRight")) { position_.x += 0.5f; }
	Vector3 offset{0.0f, 4.0f, -10.0f};
	position_.y = targetPos_.y + offset.y;
	position_.z = targetPos_.z + offset.z;
}

void CameraController::UpdateSpherical(GameEngine::InputCommand* inputCommand, GameEngine::Input* rawInput) {
	if (inputCommand && inputCommand->IsCommandAcitve("CameraMoveLeft")) { rotateMove_.x += 0.02f; }
	if (inputCommand && inputCommand->IsCommandAcitve("CameraMoveRight")) { rotateMove_.x -= 0.02f; }
	if (rawInput && rawInput->PushMouse(1)) { Vector2 delta = rawInput->GetMouseDelta(); rotateMove_.x += delta.x * mouseRotateSensitivity_; }
	if (rawInput) { Vector2 rstick = rawInput->GetRightStick(); rotateMove_.x += rstick.x * stickRotateSensitivity_; }
	rotateMove_.y = std::clamp(rotateMove_.y, 0.05f, 3.05f);
	desiredTargetLookAt_.x = targetPos_.x + kDistance_ * std::sinf(rotateMove_.y) * std::sinf(rotateMove_.x);
	desiredTargetLookAt_.y = targetPos_.y + kDistance_ * std::cosf(rotateMove_.y);
	desiredTargetLookAt_.z = targetPos_.z + kDistance_ * std::sinf(rotateMove_.y) * std::cosf(rotateMove_.x);
}

Matrix4x4 CameraController::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
	Vector3 f = Normalize(center - eye); Vector3 s = Normalize(Cross(up, f)); Vector3 u = Cross(f, s);
	Matrix4x4 result = { {{ s.x,  s.y, s.z, 0 }, { u.x,  u.y, u.z, 0 }, { f.x,  f.y, f.z, 0 }, { 0.0f, 0.0f, 0.0f, 1} } }; return result;
}