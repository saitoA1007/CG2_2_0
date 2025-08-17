#include"MyMath.h"
#include<cassert>
#include<cmath>

Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs) {

	Quaternion result;
	result.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
	result.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
	result.y = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x;
	result.z = lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w;
	return result;
}

Quaternion IdentityQuaternion() {
	return { 0.0f, 0.0f, 0.0f, 1.0f };
}

Quaternion Conjugate(const Quaternion& quaternion) {
	return { -quaternion.x, -quaternion.y, -quaternion.z, quaternion.w };
}

float Norm(const Quaternion& quaternion) {
	return std::sqrt(quaternion.x * quaternion.x + quaternion.y * quaternion.y + quaternion.z * quaternion.z + quaternion.w * quaternion.w);
}

Quaternion Normalize(const Quaternion& quaternion) {
	float norm = Norm(quaternion);
	// 0除算を避けるため単位Quaternionを返す
	if (norm == 0.0f) {
		return IdentityQuaternion();
	}
	return { quaternion.x / norm, quaternion.y / norm, quaternion.z / norm, quaternion.w / norm };
}

Quaternion Inverse(const Quaternion& quaternion) {
	float norm = Norm(quaternion);
	// 0除算を避けるため単位Quaternionを返す
	if (norm == 0.0f) {
		return IdentityQuaternion();
	}
	Quaternion conjugate = Conjugate(quaternion);
	float invNorm = 1.0f / (norm * norm);
	return { conjugate.x * invNorm, conjugate.y * invNorm,conjugate.z * invNorm,conjugate.w * invNorm };
}

Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	float halfAngle = angle / 2.0f;
	float sin = std::sin(halfAngle);
	float cos = std::cos(halfAngle);
	return { axis.x * sin, axis.y * sin, axis.z * sin, cos };
}

Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion) {
	// 四元数とベクトルの回転（q * v * q^-1）
	Quaternion r = { vector.x, vector.y, vector.z,0.0f };
	// quaternionの共役を求める
	Quaternion qConj = Conjugate(quaternion);
	Quaternion rotated = Multiply(Multiply(quaternion, r), qConj);
	return { rotated.x, rotated.y, rotated.z };
}

Matrix4x4 MakeRotateMatrix(const Quaternion& q) {
	Matrix4x4 result = {
		q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z, 2.0f * (q.x * q.y + q.w * q.z), 2.0f * (q.x * q.z - q.w * q.y), 0.0f,
		2.0f * (q.x * q.y - q.w * q.z), q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z, 2.0f * (q.y * q.z + q.w * q.x), 0.0f,
		2.0f * (q.x * q.z + q.w * q.y), 2.0f * (q.y * q.z - q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z, 0.0f,
		0.0f,0.0f,0.0f,1.0f,
	};
	return result;
}

float Dot(const Quaternion& a, const Quaternion& b) {
	return  a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t) {
	Quaternion q1Copy = q1;
	float dot = Dot(q0, q1);
	// 四元数の符号が逆だと最短経路で補間されないので反転
	if (dot < 0.0f) {
		dot = -dot;
		q1Copy = -q1;
	}
	// なす角
	float theta = std::acosf(dot);
	float sinTheta = std::sinf(theta);
	// 補間係数を求める
	float scale0 = std::sinf((1.0f - t) * theta) / sinTheta;
	float scale1 = std::sinf(t * theta) / sinTheta;
	return scale0 * q0 + scale1 * q1Copy;
}

Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle) {

	float cos = std::cosf(angle);
	float sin = std::sinf(angle);
	float t = 1.0f - cos;

	Matrix4x4 result = {
		axis.x * axis.x * t + cos, axis.x * axis.y * t + axis.z * sin, axis.x * axis.z * t - axis.y * sin, 0.0f,
		axis.x * axis.y * t - axis.z * sin, axis.y * axis.y * t + cos, axis.y * axis.z * t + axis.x * sin, 0.0f,
		axis.x * axis.z * t + axis.y * sin, axis.y * axis.z * t - axis.x * sin, axis.z * axis.z * t + cos, 0.0f,
		0.0f,0.0f,0.0f,1.0f,
	};
	return result;
}

Quaternion MakeEulerQuaternion(float pitch, float yaw, float roll) {
	// pitch==x, yaw==y, roll==z
	Quaternion qx = MakeRotateAxisAngleQuaternion({ 1,0,0 }, pitch);
	Quaternion qy = MakeRotateAxisAngleQuaternion({ 0,1,0 }, yaw);
	Quaternion qz = MakeRotateAxisAngleQuaternion({ 0,0,1 }, roll);

	// 回転順序ZYX
	return Multiply(qy, Multiply(qx, qz));
}

Matrix4x4 MakeWorldMatrixFromEulerRotation(const Vector3 position, const Vector3& rotateEuler, const Vector3& scale) {

	// 回転行列を作成
	Quaternion rotate = MakeEulerQuaternion(rotateEuler.x, rotateEuler.y, rotateEuler.z);
	Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);

	// 拡縮行列
	Matrix4x4 scaleMatrix = {
		scale.x, 0.0f,   0.0f,   0.0f,
		0.0f,   scale.y, 0.0f,   0.0f,
		0.0f,   0.0f,   scale.z, 0.0f,
		0.0f,   0.0f,   0.0f,    1.0f
	};

	// 平行移動行列
	Matrix4x4 translateMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		position.x, position.y, position.z, 1.0f
	};

	// SRT行列
	Matrix4x4 worldMatrix = Multiply(Multiply(scaleMatrix, rotateMatrix), translateMatrix);
	return worldMatrix;
}

Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

float Length(const Vector3& v) {
	return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float Length(const Vector2& v) {
	return std::sqrtf(v.x * v.x + v.y * v.y);
}

Vector3 Normalize(const Vector3& v) {
	float length = Length(v);
	if (length == 0.0f) {
		return Vector3(0.0f, 0.0f, 0.0f);
	} else {
		return Vector3(v.x / length, v.y / length, v.z / length);
	}
}

Vector2 Normalize(const Vector2& v) {
	float length = Length(v);
	if (length == 0.0f) {
		return Vector2(0.0f, 0.0f);
	} else {
		return Vector2(v.x / length, v.y / length);
	}
}

float Dot(const Vector3& v1, const Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 identity = {};
	for (int i = 0; i < 4; ++i) {
		// 対角成分を1に設定
		identity.m[i][i] = 1.0f;
	}
	return identity;
}

Matrix4x4 Multiply(const Matrix4x4& matrix1, const Matrix4x4& matrix2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = 0;
			for (int k = 0; k < 4; ++k) {
				result.m[i][j] += matrix1.m[i][k] * matrix2.m[k][j];
			}
		}
	}
	return result;
}

Matrix4x4 MakeRotateXMatrix(const float& theta) {
	Matrix4x4 result = {
		1, 0, 0, 0,
		0, std::cosf(theta), std::sinf(theta), 0,
		0, -std::sinf(theta), std::cosf(theta), 0,
		0, 0, 0, 1
	};
	return result;
}

Matrix4x4 MakeRotateYMatrix(const float& theta) {
	Matrix4x4 result = {
		std::cosf(theta), 0, -std::sinf(theta), 0,
		0, 1, 0, 0,
		std::sinf(theta), 0, std::cosf(theta), 0,
		0, 0, 0, 1
	};
	return result;
}

Matrix4x4 MakeRotateZMatrix(const float& theta) {
	Matrix4x4 result = {
		std::cosf(theta), std::sinf(theta), 0, 0,
		-std::sinf(theta), std::cosf(theta), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result = {
		scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1
	};
	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		translate.x, translate.y, translate.z, 1
	};
	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3 translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateMatrix = Multiply(MakeRotateXMatrix(rotate.x), Multiply(MakeRotateYMatrix(rotate.y), MakeRotateZMatrix(rotate.z)));
	Matrix4x4 transformMatrix = Multiply(scaleMatrix, rotateMatrix);
	Matrix4x4 result = transformMatrix;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1;
	return result;
}

Matrix4x4 InverseMatrix(const Matrix4x4& matrix) {
	Matrix4x4 result;
	float det = matrix.m[0][0] * matrix.m[1][1] * matrix.m[2][2] * matrix.m[3][3] +
		matrix.m[0][0] * matrix.m[1][2] * matrix.m[2][3] * matrix.m[3][1] +
		matrix.m[0][0] * matrix.m[1][3] * matrix.m[2][1] * matrix.m[3][2] +
		matrix.m[0][1] * matrix.m[1][0] * matrix.m[2][3] * matrix.m[3][2] +
		matrix.m[0][1] * matrix.m[1][2] * matrix.m[2][0] * matrix.m[3][3] +
		matrix.m[0][1] * matrix.m[1][3] * matrix.m[2][2] * matrix.m[3][0] +
		matrix.m[0][2] * matrix.m[1][0] * matrix.m[2][1] * matrix.m[3][3] +
		matrix.m[0][2] * matrix.m[1][1] * matrix.m[2][3] * matrix.m[3][0] +
		matrix.m[0][2] * matrix.m[1][3] * matrix.m[2][0] * matrix.m[3][1] +
		matrix.m[0][3] * matrix.m[1][0] * matrix.m[2][2] * matrix.m[3][1] +
		matrix.m[0][3] * matrix.m[1][1] * matrix.m[2][0] * matrix.m[3][2] +
		matrix.m[0][3] * matrix.m[1][2] * matrix.m[2][1] * matrix.m[3][0] -
		matrix.m[0][0] * matrix.m[1][1] * matrix.m[2][3] * matrix.m[3][2] -
		matrix.m[0][0] * matrix.m[1][2] * matrix.m[2][1] * matrix.m[3][3] -
		matrix.m[0][0] * matrix.m[1][3] * matrix.m[2][2] * matrix.m[3][1] -
		matrix.m[0][1] * matrix.m[1][0] * matrix.m[2][2] * matrix.m[3][3] -
		matrix.m[0][1] * matrix.m[1][2] * matrix.m[2][3] * matrix.m[3][0] -
		matrix.m[0][1] * matrix.m[1][3] * matrix.m[2][0] * matrix.m[3][2] -
		matrix.m[0][2] * matrix.m[1][0] * matrix.m[2][3] * matrix.m[3][1] -
		matrix.m[0][2] * matrix.m[1][1] * matrix.m[2][0] * matrix.m[3][3] -
		matrix.m[0][2] * matrix.m[1][3] * matrix.m[2][1] * matrix.m[3][0] -
		matrix.m[0][3] * matrix.m[1][0] * matrix.m[2][1] * matrix.m[3][2] -
		matrix.m[0][3] * matrix.m[1][1] * matrix.m[2][2] * matrix.m[3][0] -
		matrix.m[0][3] * matrix.m[1][2] * matrix.m[2][0] * matrix.m[3][1];
	result.m[0][0] = (matrix.m[1][1] * matrix.m[2][2] * matrix.m[3][3] + matrix.m[1][2] * matrix.m[2][3] * matrix.m[3][1] + matrix.m[1][3] * matrix.m[2][1] * matrix.m[3][2] - matrix.m[1][1] * matrix.m[2][3] * matrix.m[3][2] - matrix.m[1][2] * matrix.m[2][1] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[2][2] * matrix.m[3][1]) / det;
	result.m[0][1] = (matrix.m[0][1] * matrix.m[2][3] * matrix.m[3][2] + matrix.m[0][2] * matrix.m[2][1] * matrix.m[3][3] + matrix.m[0][3] * matrix.m[2][2] * matrix.m[3][1] - matrix.m[0][1] * matrix.m[2][2] * matrix.m[3][3] - matrix.m[0][2] * matrix.m[2][3] * matrix.m[3][1] - matrix.m[0][3] * matrix.m[2][1] * matrix.m[3][2]) / det;
	result.m[0][2] = (matrix.m[0][1] * matrix.m[1][2] * matrix.m[3][3] + matrix.m[0][2] * matrix.m[1][3] * matrix.m[3][1] + matrix.m[0][3] * matrix.m[1][1] * matrix.m[3][2] - matrix.m[0][1] * matrix.m[1][3] * matrix.m[3][2] - matrix.m[0][2] * matrix.m[1][1] * matrix.m[3][3] - matrix.m[0][3] * matrix.m[1][2] * matrix.m[3][1]) / det;
	result.m[0][3] = (matrix.m[0][1] * matrix.m[1][3] * matrix.m[2][2] + matrix.m[0][2] * matrix.m[1][1] * matrix.m[2][3] + matrix.m[0][3] * matrix.m[1][2] * matrix.m[2][1] - matrix.m[0][1] * matrix.m[1][2] * matrix.m[2][3] - matrix.m[0][2] * matrix.m[1][3] * matrix.m[2][1] - matrix.m[0][3] * matrix.m[1][1] * matrix.m[2][2]) / det;
	result.m[1][0] = (matrix.m[1][0] * matrix.m[2][3] * matrix.m[3][2] + matrix.m[1][2] * matrix.m[2][0] * matrix.m[3][3] + matrix.m[1][3] * matrix.m[2][2] * matrix.m[3][0] - matrix.m[1][0] * matrix.m[2][2] * matrix.m[3][3] - matrix.m[1][2] * matrix.m[2][3] * matrix.m[3][0] - matrix.m[1][3] * matrix.m[2][0] * matrix.m[3][2]) / det;
	result.m[1][1] = (matrix.m[0][0] * matrix.m[2][2] * matrix.m[3][3] + matrix.m[0][2] * matrix.m[2][3] * matrix.m[3][0] + matrix.m[0][3] * matrix.m[2][0] * matrix.m[3][2] - matrix.m[0][0] * matrix.m[2][3] * matrix.m[3][2] - matrix.m[0][2] * matrix.m[2][0] * matrix.m[3][3] - matrix.m[0][3] * matrix.m[2][2] * matrix.m[3][0]) / det;
	result.m[1][2] = (matrix.m[0][0] * matrix.m[1][3] * matrix.m[3][2] + matrix.m[0][2] * matrix.m[1][0] * matrix.m[3][3] + matrix.m[0][3] * matrix.m[1][2] * matrix.m[3][0] - matrix.m[0][0] * matrix.m[1][2] * matrix.m[3][3] - matrix.m[0][2] * matrix.m[1][3] * matrix.m[3][0] - matrix.m[0][3] * matrix.m[1][0] * matrix.m[3][2]) / det;
	result.m[1][3] = (matrix.m[0][0] * matrix.m[1][2] * matrix.m[2][3] + matrix.m[0][2] * matrix.m[1][3] * matrix.m[2][0] + matrix.m[0][3] * matrix.m[1][0] * matrix.m[2][2] - matrix.m[0][0] * matrix.m[1][3] * matrix.m[2][2] - matrix.m[0][2] * matrix.m[1][0] * matrix.m[2][3] - matrix.m[0][3] * matrix.m[1][2] * matrix.m[2][0]) / det;
	result.m[2][0] = (matrix.m[1][0] * matrix.m[2][1] * matrix.m[3][3] + matrix.m[1][1] * matrix.m[2][3] * matrix.m[3][0] + matrix.m[1][3] * matrix.m[2][0] * matrix.m[3][1] - matrix.m[1][0] * matrix.m[2][3] * matrix.m[3][1] - matrix.m[1][1] * matrix.m[2][0] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[2][1] * matrix.m[3][0]) / det;
	result.m[2][1] = (matrix.m[0][0] * matrix.m[2][3] * matrix.m[3][1] + matrix.m[0][1] * matrix.m[2][0] * matrix.m[3][3] + matrix.m[0][3] * matrix.m[2][1] * matrix.m[3][0] - matrix.m[0][0] * matrix.m[2][1] * matrix.m[3][3] - matrix.m[0][1] * matrix.m[2][3] * matrix.m[3][0] - matrix.m[0][3] * matrix.m[2][0] * matrix.m[3][1]) / det;
	result.m[2][2] = (matrix.m[0][0] * matrix.m[1][1] * matrix.m[3][3] + matrix.m[0][1] * matrix.m[1][3] * matrix.m[3][0] + matrix.m[0][3] * matrix.m[1][0] * matrix.m[3][1] - matrix.m[0][0] * matrix.m[1][3] * matrix.m[3][1] - matrix.m[0][1] * matrix.m[1][0] * matrix.m[3][3] - matrix.m[0][3] * matrix.m[1][1] * matrix.m[3][0]) / det;
	result.m[2][3] = (matrix.m[0][0] * matrix.m[1][3] * matrix.m[2][1] + matrix.m[0][1] * matrix.m[1][0] * matrix.m[2][3] + matrix.m[0][3] * matrix.m[1][1] * matrix.m[2][0] - matrix.m[0][0] * matrix.m[1][1] * matrix.m[2][3] - matrix.m[0][1] * matrix.m[1][3] * matrix.m[2][0] - matrix.m[0][3] * matrix.m[1][0] * matrix.m[2][1]) / det;
	result.m[3][0] = (matrix.m[1][0] * matrix.m[2][2] * matrix.m[3][1] + matrix.m[1][1] * matrix.m[2][0] * matrix.m[3][2] + matrix.m[1][2] * matrix.m[2][1] * matrix.m[3][0] - matrix.m[1][0] * matrix.m[2][1] * matrix.m[3][2] - matrix.m[1][1] * matrix.m[2][2] * matrix.m[3][0] - matrix.m[1][2] * matrix.m[2][0] * matrix.m[3][1]) / det;
	result.m[3][1] = (matrix.m[0][0] * matrix.m[2][1] * matrix.m[3][2] + matrix.m[0][1] * matrix.m[2][2] * matrix.m[3][0] + matrix.m[0][2] * matrix.m[2][0] * matrix.m[3][1] - matrix.m[0][0] * matrix.m[2][2] * matrix.m[3][1] - matrix.m[0][1] * matrix.m[2][0] * matrix.m[3][2] - matrix.m[0][2] * matrix.m[2][1] * matrix.m[3][0]) / det;
	result.m[3][2] = (matrix.m[0][0] * matrix.m[1][2] * matrix.m[3][1] + matrix.m[0][1] * matrix.m[1][0] * matrix.m[3][2] + matrix.m[0][2] * matrix.m[1][1] * matrix.m[3][0] - matrix.m[0][0] * matrix.m[1][1] * matrix.m[3][2] - matrix.m[0][1] * matrix.m[1][2] * matrix.m[3][0] - matrix.m[0][2] * matrix.m[1][0] * matrix.m[3][1]) / det;
	result.m[3][3] = (matrix.m[0][0] * matrix.m[1][1] * matrix.m[2][2] + matrix.m[0][1] * matrix.m[1][2] * matrix.m[2][0] + matrix.m[0][2] * matrix.m[1][0] * matrix.m[2][1] - matrix.m[0][0] * matrix.m[1][2] * matrix.m[2][1] - matrix.m[0][1] * matrix.m[1][0] * matrix.m[2][2] - matrix.m[0][2] * matrix.m[1][1] * matrix.m[2][0]) / det;
	return result;
}

Matrix4x4 Transpose(const Matrix4x4& matrix) {
	Matrix4x4 result;
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			result.m[y][x] = matrix.m[x][y];
		}
	}
	return result;
}

Matrix4x4 InverseTranspose(const Matrix4x4& matrix) {
	Matrix4x4 result = InverseMatrix(matrix);
	return Transpose(result);
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	float h = 1 / std::tanf(fovY / 2);
	float w = h / aspectRatio;
	Matrix4x4 result = {
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, farClip / (farClip - nearClip), 1,
		0, 0, -nearClip * farClip / (farClip - nearClip), 0
	};
	return result;
}

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {
		2.0f / (right - left), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f / (nearClip - farClip), 0.0f,
		(left + right) / (left - right), (top + bottom) / (bottom - top), nearClip / (nearClip - farClip), 1.0f
	};
	return result;
}

Vector3 Transforms(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + matrix.m[3][2];
	// 同次座標に変換するために 4D ベクトルを使う
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}