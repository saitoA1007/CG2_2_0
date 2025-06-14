#include"PostProcessGrayScale.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 main(VertexShaderOutput input) : SV_TARGET
{
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    float32_t4 color = textureColor;
    
    float32_t luminance = dot(float32_t3(0.299f, 0.587f, 0.114f), color.xyz);
    // 光度が低いと描画しないようにする
    if (luminance < 0.6f)
    {
        color = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    
    return color;
}