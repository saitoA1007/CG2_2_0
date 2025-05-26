#include"LightManager.h"
#include"EngineSource/Common/CreateBufferResource.h"
#include"EngineSource/Math/MyMath.h"
using namespace GameEngine;

void LightManager::Initialize(ID3D12Device* device) {
    // 平行光源
    directionalLight_ = std::make_unique<DirectionalLight>();
    directionalLight_->Initialize({ 1,1,1,1 }, { 0,-1,0 }, 1.0f);
    // 点光源
    pointLight_ = std::make_unique<PointLight>();
    pointLight_->Initialize({ 1,1,1,1 }, { 0,0,0 }, 1.0f);
    // スポットライト
    spotLight_ = std::make_unique<SpotLight>();
    spotLight_->Initialize({ 1,1,1,1 }, { 0,0,0 }, 1.0f);

    // 平行光源のリソースを作る。
    lightGroupResource_ = CreateBufferResource(device, sizeof(LightGroupData));
    // 書き込むためのアドレスを取得
    lightGroupResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightGroupData_));
    // デフォルト値を設定
    lightGroupData_->directionalLightData_ = directionalLight_->GetDirectionalLightData();
    lightGroupData_->pointLightData_ = pointLight_->GetPointLightData();
    lightGroupData_->spotLightData_ = spotLight_->GetSpotLightData();
}

void LightManager::Update() {
    // 更新
    lightGroupData_->directionalLightData_ = directionalLight_->GetDirectionalLightData();
    lightGroupData_->pointLightData_ = pointLight_->GetPointLightData();
    lightGroupData_->spotLightData_ = spotLight_->GetSpotLightData();
}

void LightManager::SetDirectionalData(const DirectionalLight::DirectionalLightData& directionalData) {
    directionalLight_->SetDirectionalLightData(directionalData);
}

void  LightManager::SetDirectionalDirction(const Vector3& lightdir) {
    directionalLight_->SetLightDir(lightdir);
}

void LightManager::SetDirectionalLightActive(const bool& active) {
    directionalLight_->SetActive(active);
}

void LightManager::SetPointData(const PointLight::PointLightData& pointData) {
    pointLight_->SetPointLightData(pointData);
}

void LightManager::SetPointLightPosition(const Vector3& position) {
    pointLight_->SetLightPosition(position);
}

void LightManager::SetPointLightActive(const bool& active) {
    pointLight_->SetLightActive(active);
}

void LightManager::SetSpotData(const SpotLight::SpotLightData& spotData) {
    spotLight_->SetSpotLightData(spotData);
}

void LightManager::SetSpotLightPosition(const Vector3& position) {
    spotLight_->SetLightPosition(position);
}

void LightManager::SetSpotLightActive(const bool& active) {
    spotLight_->SetLightActive(active);
}