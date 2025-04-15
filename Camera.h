#pragma once
#include"Matrix4x4.h"
#include"Vector3.h"
#include"Math.h"

class Camera {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Transform transform, int kClientWidth, int kClientHeight);

	Matrix4x4 MakeWVPMatrix(Matrix4x4 worldMatrix);

	Matrix4x4 GetViewMatrix() { return viewMatrix_; }
	Matrix4x4 GetProjectionMatrix() { return projectionMatrix_; }
	Matrix4x4 GetOrthographicMatrix() { return orthographicMatrix_; }

private:

	Matrix4x4 cameraMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 orthographicMatrix_;
	Matrix4x4 WVPMatrix_;
};