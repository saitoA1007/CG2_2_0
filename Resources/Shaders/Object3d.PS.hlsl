#include"Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t3 specularColor;
    float shininess;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct DirectionalLight
{
    float32_t4 color; // ライトの色
    float32_t3 direction; // ライトの向き
    float intensity; // 輝度
};
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct Camera
{
    float32_t3 worldPosition;
};
ConstantBuffer<Camera> gCamera : register(b2);

struct PointLight
{
    float32_t4 color; // ライトの色
    float32_t3 position; // ライトの位置
    float intensity; // 輝度
    int32_t acitce; // 有効化
    float radius; // ライトの届く最大距離
    float decay; // 減衰率
};
ConstantBuffer<PointLight> gPointLight : register(b3);

struct SpotLight
{
    float32_t4 color; // ライトの色
    float32_t3 position; // ライトの位置
    float32_t intensity; // 輝度
    float32_t3 direction; // ライトの方向
    float32_t distance; // ライトの最大距離
    float32_t decay; // 減衰率
    float32_t cosAngle; // 減衰率
    float32_t cosFalloffStart; // 
    int32_t acitce; // 有効化
};
ConstantBuffer<SpotLight> gSpotLight : register(b4);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    if (textureColor.a == 0.0)
    {
        discard;
    }
    
    if (gMaterial.enableLighting)
    { // Lightingする場合
        
        float32_t3 tmpColor = { 0.0f, 0.0f, 0.0f };
        
        // half lambert
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        // 拡散反射
        float32_t3 diffuseDirectionalLight = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
           
        // cameraDirection
        float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
        // 鏡面反射
        float32_t3 specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gMaterial.specularColor;
        
        tmpColor += diffuseDirectionalLight + specularDirectionalLight;
        
        if (gPointLight.acitce)
        {
            
            float32_t distance = length(gPointLight.position - input.worldPosition); // pointLightへの距離
            float32_t factor = pow(saturate(-distance / gPointLight.radius + 1.0), gPointLight.decay); // 指数によるコントロール
            
            // direcition
            float32_t3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);
            // half lambert
            float NdotL = dot(normalize(input.normal), -pointLightDirection);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            // diffuse
            float32_t3 diffusePointLight = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cos * gPointLight.intensity * factor;
           
            // cameraDirection
            float32_t3 halfVector = normalize(-pointLightDirection + toEye);
            float NDotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NDotH), gMaterial.shininess); // reflectintency
            // specular
            float32_t3 specularPointLight = gPointLight.color.rgb * gPointLight.intensity * specularPow * gMaterial.specularColor * factor;
            
             // diffuse+specular
            tmpColor += diffusePointLight + specularPointLight;
        }         
        
        if (gSpotLight.acitce)
        {
            // 入射光を求める
            float32_t3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSpotLight.position);
            // 角度に応じた減衰
            float32_t cosAngle = dot(spotLightDirectionOnSurface, gSpotLight.direction);
            float32_t falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle));
            // 距離減衰
            float distanceToLight = length(gSpotLight.position - input.worldPosition);
            float attenuationFactor = pow(1.0f / distanceToLight, gSpotLight.decay) * saturate(1.0f - distanceToLight / gSpotLight.distance);
            
            // half lambert
            float NdotL = dot(normalize(input.normal), -gSpotLight.direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            // diffuse
            float32_t3 diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * cos * gSpotLight.intensity * attenuationFactor * falloffFactor;
           
            // cameraDirection
            float32_t3 halfVector = normalize(-gSpotLight.direction + toEye);
            float NDotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
            // specular
            float32_t3 specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * specularPow * gMaterial.specularColor * attenuationFactor * falloffFactor;
            
            // diffuse+specular
            tmpColor += diffuseSpotLight + specularSpotLight;
        }
        
        // 最終的な色を適応
        output.color.rgb = tmpColor;
       
        // アルファ値を適応
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    { // Lighttingしない場合。
        output.color = gMaterial.color * textureColor;
    }
    
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}