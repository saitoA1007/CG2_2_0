#include"Primitive.hlsli"

float4 main(VertexShaderOutput input) : SV_TARGET
{
    return input.color;
}