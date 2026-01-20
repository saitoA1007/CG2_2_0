#include"LightManager.h"
#include"EngineSource/Common/CreateBufferResource.h"
#include"EngineSource/Math/MyMath.h"
using namespace GameEngine;

LightManager::~LightManager() {
	if (lightGroupData_) {
		lightGroupResource_->Unmap(0, nullptr);
		lightGroupData_ = nullptr;
	}
}

void LightManager::Initialize(ID3D12Device* device, const bool& isDirectionalActive, const int& activePointNum, const int& activeSpotNum) {
	// 平行光源
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize({ 1,1,1,1 }, { 0,-1,0 }, 1.0f);
	directionalLight_->SetActive(isDirectionalActive);

	// 点光源の生成
	for (int i = 0; i < kPointLightNum; ++i) {
		pointLights_[i] = std::make_unique<PointLight>();
		pointLights_[i]->Initialize({ 1,1,1,1 }, { 0,0,0 }, 1.0f);
		if (i < activePointNum) {
			pointLights_[i]->SetLightActive(true);
		} else {
			pointLights_[i]->SetLightActive(false);
		}
	}

	// スポットライトの生成
	for (int i = 0; i < kSpotLightNum; ++i) {
		spotLights_[i] = std::make_unique<SpotLight>();
		spotLights_[i]->Initialize({ 1,1,1,1 }, { 0,0,0 }, 1.0f);
		if (i < activeSpotNum) {
			spotLights_[i]->SetLightActive(true);
		} else {
			spotLights_[i]->SetLightActive(false);
		}
	}

	// 平行光源のリソースを作る。
	lightGroupResource_ = CreateBufferResource(device, sizeof(LightGroupData));
	// 書き込むためのアドレスを取得
	lightGroupResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightGroupData_));
	// デフォルト値を設定
	lightGroupData_->directionalLightData_ = directionalLight_->GetDirectionalLightData();
	// 更新
	Update();
}

void LightManager::Update() {
	// 更新
	if (lightGroupData_->directionalLightData_.active) {
		lightGroupData_->directionalLightData_ = directionalLight_->GetDirectionalLightData();
	}
	// 点光源
	for (int i = 0; i < kPointLightNum; ++i) {
		lightGroupData_->pointLightData_[i] = pointLights_[i]->GetPointLightData();
	}
	// スポットライト
	for (int i = 0; i < kSpotLightNum; ++i) {
		lightGroupData_->spotLightData_[i] = spotLights_[i]->GetSpotLightData();
	}
}

void LightManager::SetDirectionalData(const DirectionalLight::DirectionalLightData& directionalData) {
	directionalLight_->SetDirectionalLightData(directionalData);
}

void LightManager::SetDirectionalLightActive(const bool& active) {
	directionalLight_->SetActive(active);
}

void LightManager::SetPointData(const PointLight::PointLightData& pointData, const int& index) {
	pointLights_[index]->SetPointLightData(pointData);
}

void LightManager::SetPointLightActive(const bool& active, const int& index) {
	pointLights_[index]->SetLightActive(active);
}

void LightManager::SetSpotData(const SpotLight::SpotLightData& spotData, const int& index) {
	spotLights_[index]->SetSpotLightData(spotData);
}

void LightManager::SetSpotLightActive(const bool& active, const int& index) {
	spotLights_[index]->SetLightActive(active);
}