#pragma once
#include "Vector3.h"

namespace GameEngine {

	// 大きさの補間
	struct SizeOverLifeTimeModule {
		bool isEnable = false;
		Vector3 endSize = {}; // 最終的な大きさ
	};

	// 色の補間
	struct ColorOverLifeTimeModule {
		bool isEnable = false;
		Vector3 endColor = {};
	};

	// 透明度の補間
	struct AlphaOverLifeTimeModule {
		bool isEnable = false;
		float endAlpha = 0.0f;
	};
}