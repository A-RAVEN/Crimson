struct PixelShaderInput
{
    //float4 Position : SV_Position;
    //float4 Color    : COLOR;
};

float4 main(PixelShaderInput IN) : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}