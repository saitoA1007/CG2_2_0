#include"Vignetting.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float32_t intensity;
    float32_t time;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    // 周囲を0に、中心になるほど明るくなるように計算で調整
    float32_t2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    // correctだけで計算すると中心の最大値が0.0625で暗すぎるのでScaleで調整。この例では16倍にして1にしている
    float vignette = correct.x * correct.y * gMaterial.intensity;
    // とりあえず0.8乗でそれっぽくした
    vignette = saturate(pow(vignette, gMaterial.time));
    
    // 係数として乗算
    output.color.rgb *= vignette;
    
    return output;
}