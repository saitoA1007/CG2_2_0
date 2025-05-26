#include"DirectionalLight.h"
#include"EngineSource/Math/MyMath.h"
using namespace GameEngine;

void DirectionalLight::Initialize(const Vector4& color,const Vector3& direction,const float& intensity) {
	// デフォルト値を設定
	directionalLightData_.color = color;// 色
	directionalLightData_.direction = direction;// 方向
	directionalLightData_.intensity = intensity;// 輝度
	directionalLightData_.active = false;
}

void DirectionalLight::SetLightDir(const Vector3& lightdir) {
	directionalLightData_.direction = Normalize(lightdir);
}