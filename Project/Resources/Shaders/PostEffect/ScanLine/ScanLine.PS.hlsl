#include"../FullScreen.hlsli"
#define PI    3.14159265359f

Texture2D<float32_t4> gTexture[] : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float32_t interval;
    float32_t time;
    float32_t speed;
    float32_t3 lineColor;
    uint32_t textureHandle;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 texColor = gTexture[gMaterial.textureHandle].Sample(gSampler, input.texcoord);
    
    float scanline = input.texcoord.y * gMaterial.interval + gMaterial.time * gMaterial.speed;
    
    float mask = (sin(scanline * 2.0f * PI) + 1.0f) * 0.5f;
    
    output.color.rgb = lerp(texColor.rgb, gMaterial.lineColor, mask);
    float alpha = lerp(1.0f, 0.1f, mask);
    output.color.a = alpha;
    
    return output;
}