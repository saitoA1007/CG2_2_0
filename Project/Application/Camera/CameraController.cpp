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

// helper: build rotation matrix from Euler angles (pitch=x, yaw=y, roll=z)
static Matrix4x4 RotationMatrixFromEuler(const Vector3& euler) {
    float pitch = euler.x;
    float yaw = euler.y;
    float roll = euler.z;
    // forward vector from yaw/pitch
    Vector3 f;
    f.x = std::sinf(yaw) * std::cosf(pitch);
    f.y = std::sinf(pitch);
    f.z = std::cosf(yaw) * std::cosf(pitch);
    f = Normalize(f);
    Vector3 up{0.0f, 1.0f, 0.0f};
    Vector3 s = Normalize(Cross(up, f));
    Vector3 u = Cross(f, s);

    // apply roll around forward if needed
    if (std::fabs(roll) > 1e-6f) {
        float cr = std::cosf(roll);
        float sr = std::sinf(roll);
        // rotate s and u around f
        Vector3 s_rot = { s.x * cr + u.x * sr, s.y * cr + u.y * sr, s.z * cr + u.z * sr };
        Vector3 u_rot = { -s.x * sr + u.x * cr, -s.y * sr + u.y * cr, -s.z * sr + u.z * cr };
        s = s_rot; u = u_rot;
    }

    Matrix4x4 result = { {{ s.x,  s.y, s.z, 0 }, { u.x,  u.y, u.z, 0 }, { f.x,  f.y, f.z, 0 }, { 0.0f, 0.0f, 0.0f, 1} } };
    return result;
}

// helper: compute basis vectors s (right), u (up), f (forward) from Euler
static void BasisFromEuler(const Vector3& euler, Vector3& s, Vector3& u, Vector3& f) {
    float pitch = euler.x;
    float yaw = euler.y;
    float roll = euler.z;
    f.x = std::sinf(yaw) * std::cosf(pitch);
    f.y = std::sinf(pitch);
    f.z = std::cosf(yaw) * std::cosf(pitch);
    f = Normalize(f);
    Vector3 up{0.0f, 1.0f, 0.0f};
    s = Normalize(Cross(up, f));
    u = Cross(f, s);
    if (std::fabs(roll) > 1e-6f) {
        float cr = std::cosf(roll);
        float sr = std::sinf(roll);
        Vector3 s_rot = { s.x * cr + u.x * sr, s.y * cr + u.y * sr, s.z * cr + u.z * sr };
        Vector3 u_rot = { -s.x * sr + u.x * cr, -s.y * sr + u.y * cr, -s.z * sr + u.z * cr };
        s = s_rot; u = u_rot;
    }
}

void CameraController::Initialize() {
	camera_ = std::make_unique<Camera>();
	camera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},position_ }, 1280, 720);
	// 現在値を目標値に同期
	desiredTargetPos_ = targetPos_;
    desiredTargetLookAt_ = targetLookAt_;
	desiredTargetRotate_ = targetRotate_;
	desiredTargetFov_ = targetFov_;
	prevTargetPos_ = targetPos_;
	// view offset initialize
	viewOffsetEnabled_ = false;
	viewOffsetDesiredEye_ = {0.0f,0.0f,0.0f};
	viewOffsetDesiredLook_ = {0.0f,0.0f,0.0f};
	viewOffsetCurrentEye_ = {0.0f,0.0f,0.0f};
	viewOffsetCurrentLook_ = {0.0f,0.0f,0.0f};
}

void CameraController::Update(GameEngine::InputCommand* inputCommand, GameEngine::Input* rawInput) {
    if (!camera_) return;

	if (isAnimationPlaying_) {
        UpdateAnimation();
	} else {
		// ターゲット種類に応じて目標値更新
		std::visit([this](auto &&value) {
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, Vector3>) {
				UpdateTargetVector3(value);
			} else if constexpr (std::is_same_v<T, Line>) {
				UpdateTargetLine(value);
			} else if constexpr (std::is_same_v<T, std::vector<Vector3>>) {
				UpdateTargetVector3Array(value);
			}
		}, target_);

		// 座標系種類でカメラ位置追従 (補間前に回転入力反映)
		switch (cameraCoordinateType_) {
			case CameraCoodinateType::Cartesian: UpdateCartesian(inputCommand);  break;
			case CameraCoodinateType::Spherical: UpdateSpherical(inputCommand, rawInput); break;
		}
	}
	
	// スムーズ補間係数
	const float t = 0.1f;
	// 位置・回転(Euler)・FOV補間
	if (isAnimationPlaying_) {
		// アニメーション再生中はアニメーションの値をそのまま反映
		targetPos_ = desiredTargetPos_;
		targetLookAt_ = desiredTargetLookAt_;
		targetRotate_ = desiredTargetRotate_;
		targetFov_ = desiredTargetFov_;
	} else {
		targetPos_ = Lerp(targetPos_, desiredTargetPos_, t);
    	targetLookAt_ = Lerp(targetLookAt_, desiredTargetLookAt_, t);
		targetRotate_ = Lerp(targetRotate_, desiredTargetRotate_, t);
		targetFov_ = Lerp(targetFov_, desiredTargetFov_, t);
	}

    Vector3 eye;
    Vector3 center;

    if (isAnimationPlaying_ && !lookAtAnimationState_.keyframes.empty()) {
        center = targetLookAt_;
        Vector3 posKey = targetPos_;
        if (!rotateAnimationState_.keyframes.empty()) {
            Vector3 s,u,f;
            BasisFromEuler(targetRotate_, s, u, f);
            float distance = posKey.z;
            eye = center - f * distance + s * posKey.x + u * posKey.y;
        } else {
            eye = center + Vector3{ posKey.x, posKey.y, posKey.z };
        }
    } else {
        eye = targetLookAt_;
        center = targetPos_;
    }

	if (!isAnimationPlaying_) {
		ApplyAutoRotate(eye, center);
	}

	// 視点オフセットの適用（現在値へ補間）
	if (viewOffsetEnabled_) {
		float bs = std::clamp(viewOffsetBlendSpeed_, 0.0f, 1.0f);
		viewOffsetCurrentEye_ = Lerp(viewOffsetCurrentEye_, viewOffsetDesiredEye_, bs);
		viewOffsetCurrentLook_ = Lerp(viewOffsetCurrentLook_, viewOffsetDesiredLook_, bs);
		eye = { eye.x + viewOffsetCurrentEye_.x, eye.y + viewOffsetCurrentEye_.y, eye.z + viewOffsetCurrentEye_.z };
		center = { center.x + viewOffsetCurrentLook_.x, center.y + viewOffsetCurrentLook_.y, center.z + viewOffsetCurrentLook_.z };
	} else {
		// disable時はスムーズにゼロへ戻す
		float bs = std::clamp(viewOffsetBlendSpeed_, 0.0f, 1.0f);
		viewOffsetCurrentEye_ = Lerp(viewOffsetCurrentEye_, Vector3{0.0f,0.0f,0.0f}, bs);
		viewOffsetCurrentLook_ = Lerp(viewOffsetCurrentLook_, Vector3{0.0f,0.0f,0.0f}, bs);
		eye = { eye.x + viewOffsetCurrentEye_.x, eye.y + viewOffsetCurrentEye_.y, eye.z + viewOffsetCurrentEye_.z };
		center = { center.x + viewOffsetCurrentLook_.x, center.y + viewOffsetCurrentLook_.y, center.z + viewOffsetCurrentLook_.z };
	}

	// シェイク: 現在のオフセットを更新する（実際の適用はworldMatrix構築後に行う）
	if (cameraShakePower_ > 0.0f && cameraShakeElapsedTime_ < cameraShakeMaxTime_) {
		cameraShakeElapsedTime_ += FpsCounter::deltaTime;
		float tn = std::clamp(cameraShakeElapsedTime_ / std::max(cameraShakeMaxTime_, 0.0001f), 0.0f, 1.0f);
		float amp = shakeEnableDecay_ ? EaseOutQuad(cameraShakePower_, 0.0f, tn) : cameraShakePower_;
		float currentDistance = Length(center - eye);
		float distanceScale = std::clamp(kDistance_ / std::max(currentDistance, 0.0001f), 0.1f, 1.0f);
		amp *= distanceScale;

		static thread_local std::mt19937 rng{ std::random_device{}() };
		std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

		// 新しいターゲットが必要なら生成
 		if (shakeNeedNewTarget_) {
 			shakeTargetOffset_.x = (enableShakeX_ ? dist(rng) : 0.0f) * amp;
 			shakeTargetOffset_.y = (enableShakeY_ ? dist(rng) * 0.5f : 0.0f) * amp;
 			shakeTargetOffset_.z = (enableShakeZ_ ? dist(rng) : 0.0f) * amp;
 			// record start offset and reset progress
			shakeStartOffset_ = shakeCurrentOffset_;
			shakeEaseProgress_ = 0.0f;
			shakeFadingOut_ = false;
			shakeNeedNewTarget_ = false;
 		}

 		// イージングで現在値をターゲットへ近づける
        shakeEaseProgress_ = std::clamp(shakeEaseProgress_ + FpsCounter::deltaTime * shakeEaseSpeed_, 0.0f, 1.0f);
		if (shakeEaseFunc_) {
			shakeCurrentOffset_ = shakeEaseFunc_(shakeStartOffset_, shakeTargetOffset_, shakeEaseProgress_);
		} else {
			shakeCurrentOffset_ = Lerp(shakeCurrentOffset_, shakeTargetOffset_, shakeEaseProgress_);
		}

		// 目標になったら次の目標を要求する
        if (shakeEaseProgress_ >= 1.0f) {
			shakeNeedNewTarget_ = true;
		}
	} else {
		// シェイクが終了または無効な場合、オフセットをゼロへフェードアウト
		shakeEaseProgress_ = std::clamp(shakeEaseProgress_ + FpsCounter::deltaTime * shakeEaseSpeed_, 0.0f, 1.0f);
		if (shakeEaseFunc_) {
			if (!shakeFadingOut_) {
				// start fade from current offset
				shakeStartOffset_ = shakeCurrentOffset_;
				shakeEaseProgress_ = 0.0f;
				shakeFadingOut_ = true;
			}
			shakeCurrentOffset_ = shakeEaseFunc_(shakeStartOffset_, Vector3{ 0.0f,0.0f,0.0f }, shakeEaseProgress_);
		} else {
			shakeCurrentOffset_ = Lerp(shakeCurrentOffset_, Vector3{ 0.0f,0.0f,0.0f }, shakeEaseProgress_);
		}
		shakeTargetOffset_ = Vector3{ 0.0f,0.0f,0.0f };
		shakeNeedNewTarget_ = true;
		if (shakeEaseProgress_ >= 1.0f) {
			shakeCurrentOffset_ = Vector3{ 0.0f,0.0f,0.0f };
			cameraShakePower_ = 0.0f;
        }
	}

	Matrix4x4 rotateMatrix;
	// If animation provides explicit rotate keyframes, construct rotation from Euler angles
	if (isAnimationPlaying_ && !rotateAnimationState_.keyframes.empty()) {
		rotateMatrix = RotationMatrixFromEuler(targetRotate_);
	} else {
		rotateMatrix = LookAt(eye, center, { 0.0f,1.0f,0.0f });
	}
	Matrix4x4 worldMatrix = rotateMatrix;
	worldMatrix.m[3][0] = eye.x; worldMatrix.m[3][1] = eye.y; worldMatrix.m[3][2] = eye.z;

	// Apply shake offsets to final world matrix so shake is visible even during animations
	if (Length(shakeCurrentOffset_) > 0.0f) {
		switch (shakeOrigin_) {
		case ShakeOrigin::CameraPosition:
			worldMatrix.m[3][0] += shakeCurrentOffset_.x;
			worldMatrix.m[3][1] += shakeCurrentOffset_.y;
			worldMatrix.m[3][2] += shakeCurrentOffset_.z;
			break;
		case ShakeOrigin::TargetPosition: {
			// Recompute rotation so camera looks at shifted target
			Matrix4x4 shakenRotate = LookAt(eye, Vector3{ center.x + shakeCurrentOffset_.x, center.y + shakeCurrentOffset_.y, center.z + shakeCurrentOffset_.z }, {0.0f,1.0f,0.0f});
			worldMatrix = shakenRotate;
			worldMatrix.m[3][0] = eye.x; worldMatrix.m[3][1] = eye.y; worldMatrix.m[3][2] = eye.z;
			break;
		}
		case ShakeOrigin::TargetAndCameraPosition:
			worldMatrix.m[3][0] += shakeCurrentOffset_.x * 0.5f;
			worldMatrix.m[3][1] += shakeCurrentOffset_.y * 0.5f;
			worldMatrix.m[3][2] += shakeCurrentOffset_.z * 0.5f;
			// also nudge rotation slightly by looking at partially shifted center
			Matrix4x4 partialRotate = LookAt(Vector3{ worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2] }, Vector3{ center.x + shakeCurrentOffset_.x * 0.5f, center.y + shakeCurrentOffset_.y * 0.5f, center.z + shakeCurrentOffset_.z * 0.5f }, {0.0f,1.0f,0.0f});
			// Keep translation but apply new rotation axes
			partialRotate.m[3][0] = worldMatrix.m[3][0]; partialRotate.m[3][1] = worldMatrix.m[3][1]; partialRotate.m[3][2] = worldMatrix.m[3][2];
			worldMatrix = partialRotate;
			break;
		}
	}

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
	desiredTargetFov_ = std::clamp(0.45f + length * 0.5f, 0.3f, 1.0f);
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
	float factor = std::clamp(radius / kDistance_, 0.0f, 1.0f);
    desiredTargetFov_ = Lerp(0.5f, 2.0f, factor);
}

void CameraController::UpdateCartesian(GameEngine::InputCommand* inputCommand) {
    if (!inputCommand) return;
	if (inputCommand && inputCommand->IsCommandActive("CameraMoveLeft")) { position_.x -= 0.5f; }
	if (inputCommand && inputCommand->IsCommandActive("CameraMoveRight")) { position_.x += 0.5f; }
	Vector3 offset{0.0f, 4.0f, -10.0f};
	position_.y = targetPos_.y + offset.y;
	position_.z = targetPos_.z + offset.z;
}

void CameraController::UpdateSpherical(GameEngine::InputCommand* inputCommand, GameEngine::Input* rawInput) {
    if (!inputCommand || !rawInput) return;
	if (inputCommand && inputCommand->IsCommandActive("CameraMoveLeft")) { rotateMove_.x += 0.02f; }
	if (inputCommand && inputCommand->IsCommandActive("CameraMoveRight")) { rotateMove_.x -= 0.02f; }
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

void CameraController::UpdateAnimation() {
    if (!isAnimationPlaying_) return;
    animationTime_ += FpsCounter::deltaTime * animationPlaySpeed_;

    // 各アニメーション状態の現在インデックス更新
	if (!positionAnimationState_.keyframes.empty()) {
		size_t &idx = positionAnimationState_.currentIndex;
		while (idx + 1 < positionAnimationState_.keyframes.size() &&
			animationTime_ >= positionAnimationState_.keyframes[idx + 1].time) {
			idx++;
		}
	}
	if (!rotateAnimationState_.keyframes.empty()) {
		size_t &idx = rotateAnimationState_.currentIndex;
		while (idx + 1 < rotateAnimationState_.keyframes.size() &&
			animationTime_ >= rotateAnimationState_.keyframes[idx + 1].time) {
			idx++;
		}
	}
	if (!lookAtAnimationState_.keyframes.empty()) {
		size_t &idx = lookAtAnimationState_.currentIndex;
		while (idx + 1 < lookAtAnimationState_.keyframes.size() &&
			animationTime_ >= lookAtAnimationState_.keyframes[idx + 1].time) {
		 idx++;
		}
	}
	if (!fovAnimationState_.keyframes.empty()) {
		size_t &idx = fovAnimationState_.currentIndex;
		while (idx + 1 < fovAnimationState_.keyframes.size() &&
			animationTime_ >= fovAnimationState_.keyframes[idx + 1].time) {
		 idx++;
		}
	}

	// 位置補間
	if (!positionAnimationState_.keyframes.empty()) {
		const auto& keyframes = positionAnimationState_.keyframes;
		size_t idx = positionAnimationState_.currentIndex;
		if (idx + 1 < keyframes.size()) {
			float t = (animationTime_ - keyframes[idx].time) / (keyframes[idx + 1].time - keyframes[idx].time);
			t = std::clamp(t, 0.0f, 1.0f);
			// If lookAt keys are present, position keys represent camera params (x,y offsets, z=distance)
			if (!lookAtAnimationState_.keyframes.empty()) {
				desiredTargetPos_ = keyframes[idx].interpFunc(keyframes[idx].value, keyframes[idx + 1].value, t);
			} else {
				desiredTargetPos_ = keyframes[idx].interpFunc(keyframes[idx].value, keyframes[idx + 1].value, t);
			}
		} else {
			if (!lookAtAnimationState_.keyframes.empty()) {
				desiredTargetPos_ = keyframes[idx].value;
			} else {
				desiredTargetPos_ = keyframes[idx].value;
			}
		}
	} else {
		// no position keys
	}
    // 回転補間
    if (!rotateAnimationState_.keyframes.empty()) {
		const auto& keyframes = rotateAnimationState_.keyframes;
		size_t idx = rotateAnimationState_.currentIndex;
		if (idx + 1 < keyframes.size()) {
			float t = (animationTime_ - keyframes[idx].time) / (keyframes[idx + 1].time - keyframes[idx].time);
			t = std::clamp(t, 0.0f, 1.0f);
			desiredTargetRotate_ = keyframes[idx].interpFunc(keyframes[idx].value, keyframes[idx + 1].value, t);
		} else {
			desiredTargetRotate_ = keyframes[idx].value;
		}
    }
	// 注視点補間
	if (!lookAtAnimationState_.keyframes.empty()) {
		const auto& keyframes = lookAtAnimationState_.keyframes;
		size_t idx = lookAtAnimationState_.currentIndex;
		if (idx + 1 < keyframes.size()) {
			float t = (animationTime_ - keyframes[idx].time) / (keyframes[idx + 1].time - keyframes[idx].time);
			t = std::clamp(t, 0.0f, 1.0f);
			desiredTargetLookAt_ = keyframes[idx].interpFunc(keyframes[idx].value, keyframes[idx + 1].value, t);
		} else {
			desiredTargetLookAt_ = keyframes[idx].value;
		}
	}
	// FOV補間
	if (!fovAnimationState_.keyframes.empty()) {
		const auto &keyframes = fovAnimationState_.keyframes;
		size_t idx = fovAnimationState_.currentIndex;
		if (idx + 1 < keyframes.size()) {
			float t = (animationTime_ - keyframes[idx].time) / (keyframes[idx + 1].time - keyframes[idx].time);
			t = std::clamp(t, 0.0f, 1.0f);
			desiredTargetFov_ = keyframes[idx].interpFunc(keyframes[idx].value, keyframes[idx + 1].value, t);
		} else {
			desiredTargetFov_ = keyframes[idx].value;
		}
	}

	if ((!positionAnimationState_.keyframes.empty() && positionAnimationState_.currentIndex + 1 >= positionAnimationState_.keyframes.size()) &&
        (!rotateAnimationState_.keyframes.empty() && rotateAnimationState_.currentIndex + 1 >= rotateAnimationState_.keyframes.size()) &&
		(!lookAtAnimationState_.keyframes.empty() && lookAtAnimationState_.currentIndex + 1 >= lookAtAnimationState_.keyframes.size()) &&
		(!fovAnimationState_.keyframes.empty() && fovAnimationState_.currentIndex + 1 >= fovAnimationState_.keyframes.size())) {
		// target_ に基づき通常の desiredTargetPos_ を再計算する
		std::visit([this](auto &&value) {
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, Vector3>) {
				UpdateTargetVector3(value);
			} else if constexpr (std::is_same_v<T, Line>) {
				UpdateTargetLine(value);
			} else if constexpr (std::is_same_v<T, std::vector<Vector3>>) {
				UpdateTargetVector3Array(value);
			}
			}, target_);
		switch (cameraCoordinateType_) {
			case CameraCoodinateType::Cartesian:  UpdateCartesian(nullptr);  break;
			case CameraCoodinateType::Spherical: UpdateSpherical(nullptr, nullptr); break;
		}
		isAnimationPlaying_ = false; // アニメーション終了
    }
}

// 即時適用関数
void CameraController::ApplyImmediateView(const Vector3& eye, const Vector3& center, const Vector3& euler, float fov) {
	if (!camera_) return;
	// アニメーションを停止
	isAnimationPlaying_ = false;
	// ターゲットと目標を即時同期
	targetLookAt_ = eye;
	targetPos_ = center;
	targetRotate_ = euler;
	targetFov_ = fov;
	desiredTargetLookAt_ = eye;
	desiredTargetPos_ = center;
	desiredTargetRotate_ = euler;
	desiredTargetFov_ = fov;
	prevTargetPos_ = center;

	// ワールド行列を直接構築
	Matrix4x4 rotateMatrix;
	bool useEuler = (std::fabs(euler.x) > 1e-6f) || (std::fabs(euler.y) > 1e-6f) || (std::fabs(euler.z) > 1e-6f);
	if (useEuler) {
		rotateMatrix = RotationMatrixFromEuler(euler);
	} else {
		rotateMatrix = LookAt(eye, center, {0.0f,1.0f,0.0f});
	}
	Matrix4x4 worldMatrix = rotateMatrix;
	worldMatrix.m[3][0] = eye.x; worldMatrix.m[3][1] = eye.y; worldMatrix.m[3][2] = eye.z;

	camera_->SetWorldMatrix(worldMatrix);
	camera_->SetProjectionMatrix(fov, 1280, 720, 0.1f, 1000.0f);
	camera_->UpdateFromWorldMatrix();
}
