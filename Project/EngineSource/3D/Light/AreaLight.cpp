#include"AreaLight.h"
using namespace GameEngine;

void AreaLight::Initialize(const Vector4& color, const Vector3& position, const float& intensity) {
	// デフォルト値を設定
	areaLightData_.color = color;// 色
	areaLightData_.position = position;// 位置
	areaLightData_.intensity = intensity;// 輝度
	areaLightData_.right = {1.0f,0.0f,0.0f};
	areaLightData_.width = 5.0f;
	areaLightData_.up = {0.0f,1.0f,0.0f};
	areaLightData_.height = 5.0f;
	areaLightData_.distance = 10.0f;
	areaLightData_.decay = 0.5f;
	areaLightData_.active = false;
}