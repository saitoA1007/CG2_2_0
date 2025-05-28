#include"PostProcessGrayScale.hlsli"

struct Material
{
    int32_t isEnablePostEffect;
};
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 main(VertexShaderOutput input) : SV_TARGET
{
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    float32_t4 color = textureColor;
    
    if (gMaterial.isEnablePostEffect)
    {
        float32_t gray = dot(color.rgb, float3(0.299, 0.587, 0.114));
        color = float32_t4(gray, gray, gray, color.a);
    }
    
    return color;
}