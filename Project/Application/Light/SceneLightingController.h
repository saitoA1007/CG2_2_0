#pragma once
#include<string>
#include<array>
#include"LightManager.h"
#include"DirectionalLight.h"

/// <summary>
/// シーンのライトを管理する
/// </summary>
class SceneLightingController {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="device"></param>
	void Initialize(ID3D12Device* device);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ライトのリソースを取得
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* GetResource() const {return lightManager_->GetResource();}

private:

	// ライト
	std::unique_ptr<GameEngine::LightManager> lightManager_;
	// 平行光源
	GameEngine::DirectionalLight::DirectionalLightData directionalData_;
	// 点光源
	std::array<GameEngine::PointLight::PointLightData,2> pointLightDatas_;
	// スポットライト
	std::array<GameEngine::SpotLight::SpotLightData, 2> spotLightDatas_;

private: // デバック用

	std::string directinalName_ = "DirectionalLight";
	std::string pointName_ = "PointLight";
	std::string spotName_ = "SpotLight";
private:

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};