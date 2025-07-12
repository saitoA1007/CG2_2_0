#include"PostProcessGrayScale.hlsli"
#include"GaussianBlur.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct BlurParameter
{
    float32_t sigma; // ぼかしの強さ
};
ConstantBuffer<BlurParameter> gblurParameter : register(b0);

static const float32_t2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

float4 main(VertexShaderOutput input) : SV_TARGET
{
    float4 output = float4(0, 0, 0, 1);
    float weight = 0.0f;
    float32_t kernel3x3[3][3];
    
    uint32_t w, h;
    gTexture.GetDimensions(w, h);
    float32_t2 uvStepSize = float32_t2(rcp(w), rcp(h));

    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            float2 offset = kIndex3x3[x][y];
            kernel3x3[x][y] = gauss(offset.x, offset.y, gblurParameter.sigma);
            float2 sampleUV = input.texcoord + offset * uvStepSize;

            output.rgb += gTexture.Sample(gSampler, sampleUV).rgb * kernel3x3[x][y];
            weight += kernel3x3[x][y];
        }
    }

    output.rgb *= rcp(weight);

    return output;
}