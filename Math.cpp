#include"Math.h"
#include<cassert>
#include<cmath>

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
		0, cosf(theta), sinf(theta), 0,
		0, -sinf(theta), cosf(theta), 0,
		0, 0, 0, 1
	};
	return result;
}

Matrix4x4 MakeRotateYMatrix(const float& theta) {
	Matrix4x4 result = {
		cosf(theta), 0, -sinf(theta), 0,
		0, 1, 0, 0,
		sinf(theta), 0, cosf(theta), 0,
		0, 0, 0, 1
	};
	return result;
}

Matrix4x4 MakeRotateZMatrix(const float& theta) {
	Matrix4x4 result = {
		cosf(theta), sinf(theta), 0, 0,
		-sinf(theta), cosf(theta), 0, 0,
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

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& theta, const Vector3 translate) {

	Matrix4x4 rotate = Multiply(MakeRotateXMatrix(theta.x), Multiply(MakeRotateYMatrix(theta.y), MakeRotateZMatrix(theta.z)));

	Matrix4x4 result = {
		scale.x * rotate.m[0][0], rotate.m[0][1], rotate.m[0][2], 0,
		rotate.m[1][0], scale.y * rotate.m[1][1], rotate.m[1][2], 0,
		rotate.m[2][0], rotate.m[2][1], scale.z * rotate.m[2][2], 0,
		translate.x, translate.y, translate.z, 1
	};
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

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	float h = 1 / tanf(fovY / 2);
	float w = h / aspectRatio;
	Matrix4x4 result = {
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, farClip / (farClip - nearClip), 1,
		0, 0, -nearClip * farClip / (farClip - nearClip), 0
	};
	return result;
}
