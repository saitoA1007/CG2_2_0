struct TransformationMatrix
{
    float32_t4x4 World;
    float32_t4x4 WorldInverseTranspose;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct LightViewProjection
{
    float32_t4x4 lightVPMatrix;
};
ConstantBuffer<LightViewProjection> gLightVP : register(b1);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t depth : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float32_t4 worldPos = mul(input.position, gTransformationMatrix.World);
    output.position = mul(worldPos, gLightVP.lightVPMatrix);
    // 深度値を保存 (Z / W)
    output.depth = output.position.z / output.position.w;
    return output;
}
