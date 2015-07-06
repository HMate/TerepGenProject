
cbuffer SceneBuffer : register(b0)
{
	float4x4 ViewProjMx;
};
cbuffer ObjectBuffer : register(b1)
{
	float4x4 WorldMx;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float4 worldPos : POSITION;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
    VOut output;

    output.worldPos = mul(position, WorldMx);
    output.position = mul(output.worldPos, ViewProjMx);
    output.color = color;

    return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}