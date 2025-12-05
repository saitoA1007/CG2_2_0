#pragma once
#include "Vector3.h"
#include"Range.h"

namespace GameEngine {

	// 大きさの補間
	struct SizeOverLifeTimeModule {
		bool isEnable = false;
		Vector3 endSize = {}; // 最終的な大きさ
	};

	// 色の補間
	struct ColorOverLifeTimeModule {
		bool isEnable = false;
		Vector4 endColor = {0.0f,0.0f,0.0f,1.0f};
	};

	// 透明度の補間
	struct AlphaOverLifeTimeModule {
		bool isEnable = false;
		float endAlpha = 0.0f;
	};

	// 生成位置から速度を求める
	struct VelocityFromPosition {
		bool isEnable = false;
		float minVelocity = 0.0f;
		float maxVelocity = 0.0f;
	};

	struct RotateZFromVelocity {
		bool isEnable = false;
	};

	struct ScaleXYZActive {
		bool isEnable = false;
	};

}