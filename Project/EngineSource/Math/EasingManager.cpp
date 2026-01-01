#include"EasingManager.h"
#include<cmath>
#include <algorithm>
#include"EngineSource/Math/MyMath.h"

static constexpr float PI = 3.14159265358979323846f;

float Lerp(const float& start, const float& end, const float& t) {
	//return (1.0f - t) * start + t * end;
	return start + t * (end - start);
}

Vector3 Lerp(const Vector3& start, const Vector3& end, const float& t) {
	return Vector3(start.x + t * (end.x - start.x), start.y + t * (end.y - start.y) ,start.z + t * (end.z - start.z));
}

Vector4 Lerp(const Vector4& start, const Vector4& end, const float& t) {
	return Vector4(start.x + t * (end.x - start.x), start.y + t * (end.y - start.y), start.z + t * (end.z - start.z), start.w + t * (end.w - start.w));
}

Quaternion Lerp(const Quaternion& start, const Quaternion& end, const float& t) {
	return Quaternion(start.x + t * (end.x - start.x), start.y + t * (end.y - start.y), start.z + t * (end.z - start.z), start.w + t * (end.w - start.w));
}

float EaseIn(const float& t) {
	return t * t;
}

float EaseOut(const float& t) {
	return 1.0f - std::powf(1.0f - t, 2.0f);
}

float EaseInOut(const float& t) {
	if (t < 0.5f) {
		return 2.0f * t * t;
	} else {
		return 1.0f - std::powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
	}
}

Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t) {
	Quaternion q1Copy = q1;
	float dot = Dot(q0, q1);
	// 四元数の符号が逆だと最短経路で補間されないので反転
	if (dot < 0.0f) {
		dot = -dot;
		q1Copy = -q1;
	}
	// 内積が1,0fを超えないようにする
	dot = std::clamp(dot, -1.0f, 1.0f);

	if (dot > 0.9999f) {
		Quaternion result = Lerp(q0, q1Copy, t);
		return Normalize(result);
	}

	// なす角
	float theta = std::acosf(dot);
	float sinTheta = std::sinf(theta);
	// 補間係数を求める
	float scale0 = std::sinf((1.0f - t) * theta) / sinTheta;
	float scale1 = std::sinf(t * theta) / sinTheta;
	return scale0 * q0 + scale1 * q1Copy;
}

Vector3 Slerp(const Vector3& start, const Vector3& end, const float& t) {
	// 始点と終点の長さを取得
	float startMag = Length(start);
	float endMag = Length(end);

	// 正規化
	Vector3 startNorm = start;
	Vector3 endNorm = end;
	if (startMag > 1e-6f) {
		startNorm /= startMag;
	} else {
		startNorm = Vector3(0, 0, 1);
	};

	if (endMag > 1e-6f) {
		endNorm /= endMag;
	} else {
		endNorm = Vector3(0, 0, 1);
	};

	// 内積を求める
	float dot = Dot(startNorm, endNorm);
	dot = std::clamp(dot, -1.0f, 1.0f);

	Vector3 interpVec;

	if (dot > 0.9995f) {
		// 線形補間して正規化
		interpVec = Normalize(Lerp(startNorm, endNorm, t));
	} else if (dot < -0.9995f) {
		Vector3 ortho = Cross(startNorm, Vector3(0.0f, 1.0f, 0.0f));
		if (Length(ortho) < 0.01f) {
			ortho = Cross(startNorm, Vector3(1.0f, 0.0f, 0.0f));
		}
		// 回転の基準となる垂直ベクトルを正規化
		ortho = Normalize(ortho);
		float theta = PI * t;
		interpVec = startNorm * std::cosf(theta) + ortho * std::sinf(theta);

	} else {
		// Slerpの計算
		float theta = std::acosf(dot);
		float sinTheta = std::sinf(theta);

		float sinThetaFrom = std::sinf((1.0f - t) * theta);
		float sinThetaTo = std::sinf(t * theta);

		interpVec = (startNorm * sinThetaFrom + endNorm * sinThetaTo) / sinTheta;
	}

	// 長さの線形補間
	float magnitudeLerp = Lerp(startMag, endMag, t);
	return interpVec * magnitudeLerp;
}

float EaseOutBounce(float t) {
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (t < 1.0f / d1) {
		return n1 * t * t;
	} else if (t < 2.0f / d1) {
		t -= 1.5f / d1;
		return n1 * t * t + 0.75f;
	} else if (t < 2.5f / d1) {
		t -= 2.25f / d1;
		return n1 * t * t + 0.9375f;
	} else {
		t -= 2.625f / d1;
		return n1 * t * t + 0.984375f;
	}
}