#pragma once
#include"Vector2.h"
#include"Vector3.h"
#include"Matrix4x4.h"

static const double M_PI = 3.14159265358979323846;

struct Quaternion {
	float x;
	float y;
	float z;
	float w;

	Quaternion operator+(const Quaternion& other) { return { x + other.x, y + other.y, z + other.z, w + other.w }; }
	Quaternion operator*(const float& other) { return { x * other, y * other, z * other, w * other }; }
	friend Quaternion operator*(float other, const Quaternion& q) { return { q.x * other, q.y * other, q.z * other, q.w * other }; }
	Quaternion operator-() const { return { -x, -y, -z,-w }; }
};

// Quaternionの積
Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);
// 単位Quaternionを返す
Quaternion IdentityQuaternion();
// 共役Quaternionを返す
Quaternion Conjugate(const Quaternion& quaternion);
// Quaernionのnormを返す
float Norm(const Quaternion& quaternion);
// 正規化したQuaternionを返す
Quaternion Normalize(const Quaternion& quaternion);
// 逆Quaternionを返す
Quaternion Inverse(const Quaternion& quaternion);
// 任意軸回転行列を表すQuaternionの生成
Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);
// ベクトルをQuaternionで回転させた結果のベクトルを求める
Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion);
// Quaternionから回転行列を求める
Matrix4x4 MakeRotateMatrix(const Quaternion& q);
// 球面線形補間
Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t);
// 内積
float Dot(const Quaternion& a, const Quaternion& b);
// 4x4行列の任意軸回転行列の作成
Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle);


Quaternion MakeEulerQuaternion(float pitch, float yaw, float roll);

Matrix4x4 MakeWorldMatrixFromEulerRotation(const Vector3 position, const Vector3& rotateEuler, const Vector3& scale);

// 減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2);

// ベクトルの長さを求める
float Length(const Vector3& v);
float Length(const Vector2& v);
// ベクトルの正規化
Vector3 Normalize(const Vector3& v);
Vector2 Normalize(const Vector2& v);
// 内積
float Dot(const Vector3& v1, const Vector3& v2);
// 外積
Vector3 Cross(const Vector3& v1, const Vector3& v2);

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

// 4x4行列の転置
Matrix4x4 Transpose(const Matrix4x4& matrix);

// 4x4行列の逆転置行列
Matrix4x4 InverseTranspose(const Matrix4x4& matrix);

// 透視投影行列の作成
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

// 平行投射行列の作成
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

// (3+1)次元座標系をデカルト座標系に変換
Vector3 Transforms(const Vector3& vector, const Matrix4x4& matrix);
