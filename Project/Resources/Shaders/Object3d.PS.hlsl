#include"Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t3 specularColor;
    float shininess;
    uint32_t textureHandle;
};
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float32_t4> gTexture[] : register(t0);
SamplerState gSampler : register(s0);

cbuffer LightGroup : register(b1)
{
    DirectionalLight gDirectionalLight;
    PointLight gPointLight[POINTLIGHT_NUM];
    SpotLight gSpotLight[SPOTLIGHT_NUM];
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
    
    if (textureColor.a == 0.0)
    {
        discard;
    }
    
    if (gMaterial.enableLighting)
    { // Lightingする場合
        
        float32_t3 tmpColor = { 0.0f, 0.0f, 0.0f };
        // cameraDirection
        float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        
        // 平行光源
        if (gDirectionalLight.active)
        {
            // half lambert
            float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            // 拡散反射
            float32_t3 diffuseDirectionalLight = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
              
            // cameraDirection
            float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
            float NDotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
            // 鏡面反射
            float32_t3 specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gMaterial.specularColor;
        
            // diffuse+specular
            tmpColor += diffuseDirectionalLight + specularDirectionalLight;
        }
        
        // 点光源
        for (int i = 0; i < POINTLIGHT_NUM; ++i)
        {
            if (gPointLight[i].active)
            {
                float32_t distance = length(gPointLight[i].position - input.worldPosition); // pointLightへの距離
                float32_t factor = pow(saturate(-distance / gPointLight[i].radius + 1.0), gPointLight[i].decay); // 指数によるコントロール
            
                // direcition
                float32_t3 pointLightDirection = normalize(input.worldPosition - gPointLight[i].position);
                // half lambert
                float NdotL = dot(normalize(input.normal), -pointLightDirection);
                float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
                // diffuse
                float32_t3 diffusePointLight = gMaterial.color.rgb * textureColor.rgb * gPointLight[i].color.rgb * cos * gPointLight[i].intensity * factor;
           
                // cameraDirection
                float32_t3 halfVector = normalize(-pointLightDirection + toEye);
                float NDotH = dot(normalize(input.normal), halfVector);
                float specularPow = pow(saturate(NDotH), gMaterial.shininess); // reflectintency
                // specular
                float32_t3 specularPointLight = gPointLight[i].color.rgb * gPointLight[i].intensity * specularPow * gMaterial.specularColor * factor;
            
                // diffuse+specular
                tmpColor += diffusePointLight + specularPointLight;
            }
        }
        
        // ポイント光源
        for (int i = 0; i < SPOTLIGHT_NUM; ++i)
        {
            if (gSpotLight[i].active)
            {
                // 入射光を求める
                float32_t3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSpotLight[i].position);
                // 角度に応じた減衰
                float32_t cosAngle = dot(spotLightDirectionOnSurface, gSpotLight[i].direction);
                float32_t falloffFactor = saturate((cosAngle - gSpotLight[i].cosAngle) / (gSpotLight[i].cosFalloffStart - gSpotLight[i].cosAngle));
                // 距離減衰
                float distanceToLight = length(gSpotLight[i].position - input.worldPosition);
                float attenuationFactor = pow(1.0f / distanceToLight, gSpotLight[i].decay) * saturate(1.0f - distanceToLight / gSpotLight[i].distance);
            
                // half lambert
                float NdotL = dot(normalize(input.normal), -gSpotLight[i].direction);
                float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
                // diffuse
                float32_t3 diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight[i].color.rgb * cos * gSpotLight[i].intensity * attenuationFactor * falloffFactor;
           
                // cameraDirection
                float32_t3 halfVector = normalize(-gSpotLight[i].direction + toEye);
                float NDotH = dot(normalize(input.normal), halfVector);
                float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
                // specular
                float32_t3 specularSpotLight = gSpotLight[i].color.rgb * gSpotLight[i].intensity * specularPow * gMaterial.specularColor * attenuationFactor * falloffFactor;
            
                // diffuse+specular
                tmpColor += diffuseSpotLight + specularSpotLight;
            }
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