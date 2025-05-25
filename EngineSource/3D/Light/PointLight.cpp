#include"PointLight.h"
#include"EngineSource/Common/CreateBufferResource.h"
#include"EngineSource/Math/MyMath.h"
using namespace GameEngine;

void PointLight::Initialize(ID3D12Device* device, const Vector4& color, const Vector3& position, const float& intensity) {
	// 平行光源のリソースを作る。
	pointLightResource_ = CreateBufferResource(device, sizeof(PointLightData));
	// 書き込むためのアドレスを取得
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));
	// デフォルト値を設定
	pointLightData_->color = color;// 色
	pointLightData_->position = position;// 位置
	pointLightData_->intensity = intensity;// 輝度
	pointLightData_->active = true;
	pointLightData_->radius = 4.0f;
	pointLightData_->decay = 2.0f;
}

void PointLight::SetLightPosition(const Vector3& position) {
	pointLightData_->position = position;
}
