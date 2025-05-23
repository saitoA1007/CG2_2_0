#pragma once

struct Matrix4x4 {
	float m[4][4];

    Matrix4x4 operator*(const Matrix4x4& rhs) const {
        Matrix4x4 result = {};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] =
                    m[i][0] * rhs.m[0][j] +
                    m[i][1] * rhs.m[1][j] +
                    m[i][2] * rhs.m[2][j] +
                    m[i][3] * rhs.m[3][j];
            }
        }
        return result;
    }
};