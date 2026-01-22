#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include <array>
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include"AreaLight.h"

namespace GameEngine {

	class LightManager {
	public:
		// 点光源の数
		static const int kPointLightNum = 3;
		// スポットライトの数
		static const int kSpotLightNum = 3;
		// 面光源の数
		static const int kAreaLightNum = 3;

		// 定数バッファ
		struct LightGroupData {
			DirectionalLight::DirectionalLightData directionalLightData_;
			PointLight::PointLightData pointLightData_[kPointLightNum];
			SpotLight::SpotLightData spotLightData_[kSpotLightNum];
			AreaLight::AreaLightData areaLightData_[kAreaLightNum];
		};

	public:
		LightManager() = default;
		~LightManager();

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="device"></param>
		void Initialize(ID3D12Device* device, const bool& isDirectionalActive, const int& activePointNum, const int& activeSpotNum, const int& activeAreaNum);

		/// <summary>
		/// 更新処理
		/// </summary>
		void Update();

	public:

		/// <summary>
		/// リソース
		/// </summary>
		/// <returns></returns>
		ID3D12Resource* GetResource()const { return lightGroupResource_.Get(); }

		/// <summary>
		/// 平行光源のデータ設定
		/// </summary>
		/// <param name="directionalData"></param>
		void SetDirectionalData(const DirectionalLight::DirectionalLightData& directionalData);

		/// <summary>
		/// 平行光源の有効化
		/// </summary>
		/// <param name="active"></param>
		void SetDirectionalLightActive(const bool& active);

		/// <summary>
		/// 点光源のデータ設定
		/// </summary>
		/// <param name="pointData"></param>
		void SetPointData(const PointLight::PointLightData& pointData, const int& index);

		/// <summary>
		/// 点光源の有効化
		/// </summary>
		/// <param name="active"></param>
		void SetPointLightActive(const bool& active, const int& index);

		/// <summary>
		/// スポットライトのデータ設定
		/// </summary>
		/// <param name="spotData"></param>
		void SetSpotData(const SpotLight::SpotLightData& spotData, const int& index);

		/// <summary>
		/// スポットライトの有効化
		/// </summary>
		/// <param name="active"></param>
		void SetSpotLightActive(const bool& active, const int& index);

		/// <summary>
		/// 面光源のデータ設定
		/// </summary>
		/// <param name="spotData"></param>
		/// <param name="index"></param>
		void SetAreaData(const AreaLight::AreaLightData& areaData, const int& index);

		/// <summary>
		/// 面光源の有効化
		/// </summary>
		/// <param name="active"></param>
		/// <param name="index"></param>
		void SetAreaLightActive(const bool& active, const int& index);

		std::unique_ptr<DirectionalLight> directionalLight_;
		std::array<std::unique_ptr<PointLight>, kPointLightNum> pointLights_;
		std::array<std::unique_ptr<SpotLight>, kSpotLightNum> spotLights_;
		std::array<std::unique_ptr<AreaLight>, kAreaLightNum> areaLights_;
	private:
		// リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> lightGroupResource_;

		// 平行光源のデータを作る
		LightGroupData* lightGroupData_ = nullptr;
	};

}
