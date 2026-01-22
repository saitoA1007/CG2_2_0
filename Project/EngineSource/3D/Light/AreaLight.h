#pragma once
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector3.h"
#include<cstdint>

namespace GameEngine {

	class AreaLight {
	public:

		struct AreaLightData {
			Vector4 color; // ライトの色
			Vector3 position; // ライトの位置
			float intensity; // 輝度
			Vector3 right; // ライトの右ベクトル
			float width; // ライトの幅
			Vector3 up; // ライトの上ベクトル
			float height; // ライトの高さ
			float distance; // ライトの届く最大距離
			float decay; // 減衰率
			int32_t active; // 有効化
			float pad;
		};

	public:
		AreaLight() = default;
		~AreaLight() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(const Vector4& color, const Vector3& position, const float& intensity);

		/// <summary>
		/// ライトの有効化
		/// </summary>
		/// <param name="actice"></param>
		void SetLightActive(const bool& actice) { areaLightData_.active = actice; }

		/// <summary>
		/// ライトデータを適応
		/// </summary>
		/// <param name="pointLightData"></param>
		void SetPointLightData(const AreaLightData& areaLightData) { areaLightData_ = areaLightData; }

		AreaLightData& GetPointLightData() { return areaLightData_; }

	private:

		// 平行光源のデータを作る
		AreaLightData areaLightData_;
	};
}