#pragma once
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector3.h"
#include<cstdint>

namespace GameEngine {
	class DirectionalLight {
	public:

		struct DirectionalLightData {
			Vector4 color; // ライトの色
			Vector3 direction; // ライトの向き
			float intensity; // 輝度
			int32_t active; // ライトの使用
			float padding[3];
		};

	public:
		DirectionalLight() = default;
		~DirectionalLight() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(const Vector4& color, const Vector3& direction, const float& intensity);

		/// <summary>
		/// ライト方向をセット
		/// </summary>
		/// <param name="lightdir">ライト方向</param>
		void SetLightDir(const Vector3& lightdir);

		/// <summary>
		/// ライト方向を取得
		/// </summary>
		/// <returns>ライト方向</returns>
		const Vector3& GetLightDir() const { return directionalLightData_.direction; }

		/// <summary>
		/// ライト色をセット
		/// </summary>
		/// <param name="lightcolor">ライト色</param>
		void SetLightColor(const Vector4& lightcolor) { directionalLightData_.color = lightcolor; }

		/// <summary>
		/// ライト色を取得
		/// </summary>
		/// <returns>ライト色</returns>
		const Vector4& GetLightColor() const { return directionalLightData_.color; }

		/// <summary>
		/// ライトの明度をセット
		/// </summary>
		/// <param name="lightIntensity"></param>
		void SetLightIntensity(const float& lightIntensity) { directionalLightData_.intensity = lightIntensity; }

		/// <summary>
		/// ライトの明度を取得
		/// </summary>
		/// <returns></returns>
		const float GetLightIntensity() const { return directionalLightData_.intensity; }

		/// <summary>
		/// 有効化
		/// </summary>
		/// <param name="active"></param>
		void SetActive(const bool& active) { directionalLightData_.active = active; }

		/// <summary>
		/// ライトデータを適応
		/// </summary>
		/// <param name="directionalLightData"></param>
		void SetDirectionalLightData(const DirectionalLightData& directionalLightData) { directionalLightData_ = directionalLightData; }

		DirectionalLightData& GetDirectionalLightData() { return directionalLightData_;}

	private:

		// 平行光源のデータを作る
		DirectionalLightData directionalLightData_;
	};
}