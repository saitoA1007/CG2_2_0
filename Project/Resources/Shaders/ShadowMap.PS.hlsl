struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t depth : TEXCOORD0;
};

struct PixelShaderOutput
{
    float32_t depth : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    // 深度値を出力
    output.depth = input.depth;
    return output;
}
