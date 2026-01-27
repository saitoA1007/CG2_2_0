#include"DirectionalLight.h"
#include"EngineSource/Math/MyMath.h"
#include<cmath>
using namespace GameEngine;

void DirectionalLight::Initialize(const Vector4& color,const Vector3& direction,const float& intensity) {
	// デフォルト値を設定
	directionalLightData_.color = color;// 色
	directionalLightData_.direction = direction;// 方向
	directionalLightData_.intensity = intensity;// 輝度
	directionalLightData_.active = false;
	directionalLightData_.isDepthTexture = 0;
}

void DirectionalLight::SetLightDir(const Vector3& lightdir) {
	directionalLightData_.direction = Normalize(lightdir);
}

void DirectionalLight::CreateDirectionalShadowMatrix(const Vector3& targetCenter,float shadowRange) {
    // ライトの方向を正規化
    Vector3 lightDir = Normalize(directionalLightData_.direction);
   
    // 基本となる View行列 の作成
    // まずはスナップなしで、ターゲット中心にライトを配置
    // ターゲットより十分手前から奥までカバーするため、距離をとる
    float distance = shadowRange * 2.0f;
    Vector3 lightPos = targetCenter + (lightDir * distance);
   
    // Upベクトルの対策
    Vector3 up = Vector3(0, 1, 0);
    if (std::abs(lightDir.y) > 0.999f) {
        // ライトがほぼ真上・真下を向いているときは、UpをX軸などにする
        up = Vector3(0, 0, 1);
    }
   
    Matrix4x4 viewMatrix = LookAt(lightPos, targetCenter, up);
   
    // Projection行列の作成
    float r = shadowRange;
    float l = -shadowRange;
    float t = shadowRange;
    float b = -shadowRange;
    float nearPlane = 0.1f;             // 0より少し大きく
    float farPlane = distance * 2.5f;   // 十分な奥行きを確保
    Matrix4x4 projMatrix = MakeOrthographicMatrix(l, t, r, b, nearPlane, farPlane);
   
    // シャドウマップのチラつきを補正
    // 基準点(0,0,0)がシャドウマップ上のどこに落ちるか計算する
    Matrix4x4 vpMatrix = viewMatrix * projMatrix;
   
    // ワールド原点をシャドウマップ空間へ変換
    Vector3 shadowOrigin = { 0.0f, 0.0f, 0.0f};
    shadowOrigin = Transforms(shadowOrigin, vpMatrix);
   
    // シャドウマップのサイズ
    float shadowMapSize = 2048.0f;
    // シャドウマップの1テクセルあたりの大きさ
    float texelSize = 2.0f / shadowMapSize;
   
    // ズレを計算
    float offsetX = shadowOrigin.x - (std::floor(shadowOrigin.x / texelSize) * texelSize);
    float offsetY = shadowOrigin.y - (std::floor(shadowOrigin.y / texelSize) * texelSize);
   
    // 射影行列をテクセル分だけ逆にずらす
    projMatrix.m[3][0] -= offsetX;
    projMatrix.m[3][1] -= offsetY;
   
    directionalLightData_.vpMatrix = viewMatrix * projMatrix;
}