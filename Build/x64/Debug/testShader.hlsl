
[[vk::binding(0, 0)]]
cbuffer testBuffer
{
    float4x4 testMatrix;
};

struct VertexToFragment
{
    float4 position : SV_POSITION;
    [[vk::location(0)]] float3 color : COLOR;
};

struct VertexInput
{
    [[vk::location(0)]] float2 position : POSITION;
    [[vk::location(1)]] float3 color : COLOR; 
};

VertexToFragment vert(in VertexInput input)
{
    VertexToFragment result = (VertexToFragment)0;
    result.position = mul(testMatrix, float4(input.position, 0, 1));
    result.color = input.color;
    return result;
}

[[vk::location(0)]] float4 frag(in VertexToFragment input) : SV_TARGET0
{
    return float4(input.color, 1.0);
}