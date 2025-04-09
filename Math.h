#pragma once
#include"Vector3.h"
#include "Matrix4x4.h"

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

// 単位行列を生成する関数
Matrix4x4 MakeIdentity4x4();

// 4x4行列の積
Matrix4x4 Multiply(const Matrix4x4& matrix1, const Matrix4x4& matrix2);

// 4xx4のX軸の回転行列を作成
Matrix4x4 MakeRotateXMatrix(const float& theta);

// 4x4のY軸の回転行列を作成
Matrix4x4 MakeRotateYMatrix(const float& theta);

// 4x4のZ軸の回転行列を作成
Matrix4x4 MakeRotateZMatrix(const float& theta);

// 4x4の拡縮行列の作成
Matrix4x4 MakeScaleMatrix(const Vector3& scale);

// 4x4の平行移動行列の作成
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

// 4x4のSRTによるアフィン変換行列の作成
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& theta, const Vector3 translate);

// 4x4逆行列の計算
Matrix4x4 InverseMatrix(const Matrix4x4& matrix);

// 透視投影行列の作成
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);