#pragma once
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector3.h"
#include <d3d12.h>
#include <wrl.h>
#include<cstdint>

namespace GameEngine {

	class PointLight {
	public:

		struct PointLightData {
			Vector4 color; // ライトの色
			Vector3 position; // ライトの位置
			float intensity; // 輝度
			int32_t active; // ライトの使用
			float radius; // ライトの届く最大距離
			float decay; // 減衰率
			float pad;
		};

	public:
		PointLight() = default;
		~PointLight() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(ID3D12Device* device, const Vector4& color, const Vector3& direction, const float& intensity);

		inline ID3D12Resource* GetResource() const { return pointLightResource_.Get(); }

		/// <summary>
		/// ライト位置をセット
		/// </summary>
		/// <param name="lightdir">ライト方向</param>
		void SetLightPosition(const Vector3& position);

		/// <summary>
		/// ライト位置を取得
		/// </summary>
		/// <returns>ライト方向</returns>
		const Vector3& GetLightPosition() const { return pointLightData_->position; }

		/// <summary>
		/// ライト色をセット
		/// </summary>
		/// <param name="lightcolor">ライト色</param>
		void SetLightColor(const Vector4& lightcolor) { pointLightData_->color = lightcolor; }

		/// <summary>
		/// ライト色を取得
		/// </summary>
		/// <returns>ライト色</returns>
		const Vector4& GetLightColor() const { return pointLightData_->color; }

		/// <summary>
		/// ライトの明度をセット
		/// </summary>
		/// <param name="lightIntensity"></param>
		void SetLightIntensity(const float& lightIntensity) { pointLightData_->intensity = lightIntensity; }

		/// <summary>
		/// ライトの明度を取得
		/// </summary>
		/// <returns></returns>
		const float GetLightIntensity() const { return pointLightData_->intensity; }

		/// <summary>
		/// ライトの有効化を設定
		/// </summary>
		/// <param name="actice"></param>
		void SetLightActive(const bool& actice) { pointLightData_->active = actice; }

		void SetRadius(const float& radius) { pointLightData_->radius = radius; }

		void SetDecay(const float& decay) { pointLightData_->decay = decay; }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_;

		// 平行光源のデータを作る
		PointLightData* pointLightData_ = nullptr;
	};
}