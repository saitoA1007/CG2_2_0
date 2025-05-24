#include"Camera.h"
#include"EngineSource/Math/MyMath.h"
#include"EngineSource/Common/CreateBufferResource.h"

using namespace GameEngine;

void Camera::Initialize(Transform transform, int kClientWidth, int kClientHeight, ID3D12Device* device) {
	// Matrixの初期化
	cameraMatrix_ = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix_ = InverseMatrix(cameraMatrix_);
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, static_cast<float>(kClientWidth) / static_cast<float>(kClientHeight), 0.1f, 200.0f);
	VPMatrix_ = Multiply(viewMatrix_, projectionMatrix_);

	if (device) {
		// カメラリソースを作成
		cameraResource_ = CreateBufferResource(device, sizeof(CameraForGPU));
		// データを書き込む
		// 書き込むためのアドレスを取得
		cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPU_));
		// 単位行列を書き込んでおく
		cameraForGPU_->worldPosition = GetWorldPosition();
	}
}

void Camera::Update() {
	VPMatrix_ = Multiply(viewMatrix_, projectionMatrix_);

	if (cameraForGPU_) {
		cameraForGPU_->worldPosition = GetWorldPosition();
	}	
}

void Camera::SetCameraPosition(const Transform& transform) {
	cameraMatrix_ = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix_ = InverseMatrix(cameraMatrix_);
	VPMatrix_ = Multiply(viewMatrix_, projectionMatrix_);

	if (cameraForGPU_) {
		cameraForGPU_->worldPosition = GetWorldPosition();
	}
}

Matrix4x4 Camera::MakeWVPMatrix(Matrix4x4 worldMatrix) {
	WVPMatrix_ = Multiply(worldMatrix, Multiply(viewMatrix_, projectionMatrix_));
	return WVPMatrix_;
}

void Camera::SetProjectionMatrix(float fovY, int kClientWidth, int kClientHeight, float nearPlane, float farPlane) {
	this->projectionMatrix_ = MakePerspectiveFovMatrix(fovY, static_cast<float>(kClientWidth) / static_cast<float>(kClientHeight), nearPlane, farPlane);
}

void Camera::SetViewMatrix(const Matrix4x4& viewMatrix) {
	this->viewMatrix_ = viewMatrix;
}

Vector3 Camera::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = cameraMatrix_.m[3][0];
	worldPos.y = cameraMatrix_.m[3][1];
	worldPos.z = cameraMatrix_.m[3][2];
	return worldPos;
}