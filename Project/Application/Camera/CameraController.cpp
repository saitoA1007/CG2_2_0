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
	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},position_ }, 1280, 720);

	// 目標値を現在値に合わせる
	desiredTargetPos_ = targetPos_;
	desiredTargetRotate_ = targetRotate_;
	desiredTargetFov_ = targetFov_;
}

void CameraController::Update(GameEngine::InputCommand* inputCommand) {
    inputCommand_ = inputCommand;

	// Target毎にdesiredTarget* を更新
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

	// 目標に向けてスムーズに補間
	const float t = 0.1f;
	targetPos_    = Lerp(targetPos_,    desiredTargetPos_,    t);
	targetRotate_ = Lerp(targetRotate_, desiredTargetRotate_, t);
	targetFov_    = Lerp(targetFov_,    desiredTargetFov_,    t);

	// 回転行列に変換(ターゲットへ注視)
	Matrix4x4 rotateMatrix = LookAt(position_, targetPos_, { 0.0f,1.0f,0.0f });

	// ワールド行列
	Matrix4x4 worldMatrix = rotateMatrix;
	worldMatrix.m[3][0] = position_.x;
	worldMatrix.m[3][1] = position_.y;
	worldMatrix.m[3][2] = position_.z;

	// ワールド行列を設定
	camera_->SetWorldMatrix(worldMatrix);
	// FOVも補間済みの値で更新(度->ラジアン)
	float fovRad = static_cast<float>(targetFov_ * static_cast<float>(M_PI) / 180.0f);
	camera_->SetProjectionMatrix(fovRad, 1280, 720, 0.1f, 1000.0f);
	// ワールド行列から更新する
	camera_->UpdateFromWorldMatrix();
}

void CameraController::UpdateTargetVector3(const Vector3& v) {
	desiredTargetPos_ = v;
	// 座標系種類でカメラ位置を更新
	switch (cameraCoordinateType_) {
		case CameraCoodinateType::Cartesian:
			UpdateCartesian(inputCommand_);
			break;
		case CameraCoodinateType::Spherical:
			UpdateSpherical(inputCommand_);
			break;
	}
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
	if (arr.empty()) {
		UpdateTargetVector3({0.0f,0.0f,0.0f});
		return;
	}
	Vector3 sum{0.0f,0.0f,0.0f};
	for (const auto& p : arr) {
		sum.x += p.x; sum.y += p.y; sum.z += p.z;
	}
	desiredTargetPos_.x = sum.x / static_cast<float>(arr.size());
	desiredTargetPos_.y = sum.y / static_cast<float>(arr.size());
	desiredTargetPos_.z = sum.z / static_cast<float>(arr.size());
	desiredTargetRotate_ = {0.0f,0.0f,0.0f};
	float maxDistSq = 0.0f;
	for (const auto& p : arr) {
		float dx = p.x - desiredTargetPos_.x;
		float dy = p.y - desiredTargetPos_.y;
		float dz = p.z - desiredTargetPos_.z;
		float d2 = dx*dx + dy*dy + dz*dz;
		if (d2 > maxDistSq) { maxDistSq = d2; }
	}
	float radius = std::sqrt(maxDistSq);
	desiredTargetFov_ = std::clamp(45.0f + radius * 0.8f, 30.0f, 90.0f);
}

void CameraController::UpdateCartesian(GameEngine::InputCommand* inputCommand) {
	if (inputCommand->IsCommandAcitve("CameraMoveLeft")) {
		position_.x -= 0.5f;
	}
	if (inputCommand->IsCommandAcitve("CameraMoveRight")) {
		position_.x += 0.5f;
	}
	Vector3 offset{0.0f, 4.0f, -10.0f};
	position_.y = targetPos_.y + offset.y;
	position_.z = targetPos_.z + offset.z;
}

void CameraController::UpdateSpherical(GameEngine::InputCommand* inputCommand) {
	if (inputCommand->IsCommandAcitve("CameraMoveLeft")) {
		rotateMove_.x += 0.02f;
	}
	if (inputCommand->IsCommandAcitve("CameraMoveRight")) {
		rotateMove_.x -= 0.02f;
	}
	position_.x = targetPos_.x + kDistance_ * std::sinf(rotateMove_.y) * std::sinf(rotateMove_.x);
	position_.y = targetPos_.y + kDistance_ * std::cosf(rotateMove_.y);
	position_.z = targetPos_.z + kDistance_ * std::sinf(rotateMove_.y) * std::cosf(rotateMove_.x);
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