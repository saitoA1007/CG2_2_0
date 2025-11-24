#include"IceMaterial.hlsli"

struct Material
{
    float32_t4 color;
    float32_t4x4 uvTransform;
    float32_t3 specularColor;
    float shininess;
    uint32_t textureHandle;
    uint32_t normalTextureHandle;
    float time;
};
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float32_t4> gTexture[] : register(t0);
SamplerState gSampler : register(s0);

cbuffer LightGroup : register(b1)
{
    DirectionalLight gDirectionalLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
};

struct Camera
{
    float32_t3 worldPosition;
};
ConstantBuffer<Camera> gCamera : register(b2);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture[gMaterial.textureHandle].Sample(gSampler, transformedUV.xy);
    
    //===========================
    // ノーマルマッピング
    //===========================
    // ノーマルマップから法線データを取得
    float32_t4 normalMapColor = gTexture[gMaterial.normalTextureHandle].Sample(gSampler, transformedUV.xy);
    
    // [0,1]範囲を[-1,1]範囲にリマップ
    float32_t3 textureNormal = normalMapColor.rgb * 2.0f - 1.0f;
    
    // TBN行列（Tangent, Binormal, Normal）を構築
    // 法線と接線の正規化
    float32_t3 normal = normalize(input.normal);
    float32_t3 tangent = normalize(input.tangent);

    // 従法線 (Binormal) の計算
    float32_t3 binormal = normalize(cross(normal, tangent));

    // 接空間からワールド空間への変換行列
    float32_t3x3 tbn = float32_t3x3(tangent, binormal, normal);

    // 法線を変換
    float32_t3 inputNormal = normalize(mul(textureNormal, tbn));
    
    //==========================
    // 平行光源の計算
    //==========================
    
    float32_t3 tmpColor = { 0.0f, 0.0f, 0.0f };
    // cameraDirection
    float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    
    // half lambert
    float NdotL = dot(inputNormal, -gDirectionalLight.direction);
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    // 拡散反射
    float32_t3 diffuseDirectionalLight = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
              
    // cameraDirection
    float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
    float NDotH = dot(inputNormal, halfVector);
    float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
    // 鏡面反射
    float32_t3 specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gMaterial.specularColor;
        
    // diffuse+specular
    tmpColor += diffuseDirectionalLight + specularDirectionalLight;
    
    // 最終的な色を適応
    output.color.rgb = tmpColor;
       
   // アルファ値を適応
    output.color.a = gMaterial.color.a * textureColor.a;
    
    if (output.color.a == 0.0)
    {
        discard;
    }
    return output;
}