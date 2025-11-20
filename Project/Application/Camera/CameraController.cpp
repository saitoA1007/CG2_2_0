#include"CameraController.h"
#include"MyMath.h"
#include <algorithm>
#include <cmath>
#include "EasingManager.h"
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
}

void CameraController::Update(GameEngine::InputCommand* inputCommand) {
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

	// 座標系種類でカメラ位置追従 (補間後 targetPos_ を基準)
	switch (cameraCoordinateType_) {
	case CameraCoodinateType::Cartesian:  UpdateCartesian(inputCommand);  break;
	case CameraCoodinateType::Spherical: UpdateSpherical(inputCommand); break;
	}
	// 位置・回転(Euler)・FOV補間
	targetPos_ = Lerp(targetPos_, desiredTargetPos_, t);
    targetLookAt_ = Lerp(targetLookAt_, desiredTargetLookAt_, t);
	targetRotate_ = Lerp(targetRotate_, desiredTargetRotate_, t);
	targetFov_ = Lerp(targetFov_, desiredTargetFov_, t);

    Matrix4x4 rotateMatrix = LookAt(targetLookAt_, targetPos_, { 0.0f,1.0f,0.0f });
	Matrix4x4 worldMatrix = rotateMatrix;
	worldMatrix.m[3][0] = targetLookAt_.x;
	worldMatrix.m[3][1] = targetLookAt_.y;
	worldMatrix.m[3][2] = targetLookAt_.z;
	camera_->SetWorldMatrix(worldMatrix);

	float fovRad = static_cast<float>(targetFov_ * static_cast<float>(M_PI) / 180.0f);
	camera_->SetProjectionMatrix(fovRad, 1280, 720, 0.1f, 1000.0f);
	camera_->UpdateFromWorldMatrix();
}

void CameraController::UpdateTargetVector3(const Vector3& v) {
	desiredTargetPos_ = v;
	// ターゲット方向から Euler を計算
	Vector3 dir = Normalize(v - position_);
	float yaw = std::atan2(dir.x, dir.z);
	float pitch = std::asin(std::clamp(dir.y, -1.0f, 1.0f));
	desiredTargetRotate_ = { pitch, yaw, 0.0f };
	desiredTargetFov_ = 60.0f;
}

void CameraController::UpdateTargetLine(const Line& line) {
	Vector3 dir = Normalize(line.diff);
	float lookDistance = 5.0f;
	desiredTargetPos_ = line.origin + dir * lookDistance;
	float yaw = std::atan2(dir.x, dir.z);
	float pitch = std::asin(std::clamp(dir.y, -1.0f, 1.0f));
	desiredTargetRotate_ = { pitch, yaw, 0.0f };
	float length = std::sqrt(line.diff.x*line.diff.x + line.diff.y*line.diff.y + line.diff.z*line.diff.z);
	desiredTargetFov_ = std::clamp(45.0f + length * 0.5f, 30.0f, 90.0f);
}

void CameraController::UpdateTargetVector3Array(const std::vector<Vector3>& arr) {
	if (arr.empty()) { UpdateTargetVector3({0.0f,0.0f,0.0f}); return; }
	Vector3 sum{0.0f,0.0f,0.0f};
	for (const auto& p : arr) { sum.x += p.x; sum.y += p.y; sum.z += p.z; }
	Vector3 avg{ sum.x / (float)arr.size(), sum.y / (float)arr.size(), sum.z / (float)arr.size() };
	desiredTargetPos_ = avg;
	Vector3 dir = Normalize(avg - position_);
	float yaw = std::atan2(dir.x, dir.z);
	float pitch = std::asin(std::clamp(dir.y, -1.0f, 1.0f));
	desiredTargetRotate_ = { pitch, yaw, 0.0f };
	float maxDistSq = 0.0f;
	for (const auto& p : arr) { float dx = p.x - avg.x; float dy = p.y - avg.y; float dz = p.z - avg.z; float d2 = dx*dx + dy*dy + dz*dz; if (d2 > maxDistSq) maxDistSq = d2; }
	float radius = std::sqrt(maxDistSq);
	desiredTargetFov_ = std::clamp(45.0f + radius * 0.8f, 30.0f, 90.0f);
}

void CameraController::UpdateCartesian(GameEngine::InputCommand* inputCommand) {
	if (inputCommand && inputCommand->IsCommandAcitve("CameraMoveLeft")) { position_.x -= 0.5f; }
	if (inputCommand && inputCommand->IsCommandAcitve("CameraMoveRight")) { position_.x += 0.5f; }
	Vector3 offset{0.0f, 4.0f, -10.0f};
	position_.y = targetPos_.y + offset.y;
	position_.z = targetPos_.z + offset.z;
}

void CameraController::UpdateSpherical(GameEngine::InputCommand* inputCommand) {
	if (inputCommand && inputCommand->IsCommandAcitve("CameraMoveLeft")) { rotateMove_.x += 0.02f; }
	if (inputCommand && inputCommand->IsCommandAcitve("CameraMoveRight")) { rotateMove_.x -= 0.02f; }
	desiredTargetLookAt_.x = targetPos_.x + kDistance_ * std::sinf(rotateMove_.y) * std::sinf(rotateMove_.x);
	desiredTargetLookAt_.y = targetPos_.y + kDistance_ * std::cosf(rotateMove_.y);
	desiredTargetLookAt_.z = targetPos_.z + kDistance_ * std::sinf(rotateMove_.y) * std::cosf(rotateMove_.x);
}

Matrix4x4 CameraController::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
	Vector3 f = Normalize(center - eye);
	Vector3 s = Normalize(Cross(up, f));
	Vector3 u = Cross(f, s);
	Matrix4x4 result = { {
		{ s.x,  s.y, s.z, 0 },
		{ u.x,  u.y, u.z, 0 },
		{ f.x,  f.y, f.z, 0 },
		{ 0.0f, 0.0f, 0.0f, 1}
	} };
	return result;
}