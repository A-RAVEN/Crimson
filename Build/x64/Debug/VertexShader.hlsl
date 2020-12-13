
struct VertexData
{
    float3 Position : POSITION;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexData IN)
{
    VertexShaderOutput OUT;
 
    OUT.Position = float4(IN.Position, 1.0f);
    return OUT;
}