
cbuffer SceneBuffer : register(b0)
{
	float4x4 ViewProjMx;
	float4x4 ViewMx;
};
cbuffer ObjectBuffer : register(b1)
{
	float4x4 WorldMx;
};

struct VIn
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

struct VOut
{
    float4 screenPos : SV_POSITION;
    linear float3 normal : NORMAL;
    float4 color : COLOR;
    float4 worldPos : POSITION;
};

VOut VShader(VIn input)
{
    VOut output;

    output.worldPos = mul(float4(input.position, 1.0), WorldMx);
    output.screenPos = mul(output.worldPos, ViewProjMx);
    
    // NOTE: normal transformation needs the transpose of the inverse of WorldMX
    // HLSL dont have inverse, but if WorldMX is orthogonal, we dont need it.
    // just use the worldmx and normalize the vector.
    // WorldMX is orthogonal, if scaling is uniform.
    // SOURCE: http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
    // output.normal = normalize(input.normal.xyz);
    output.normal = normalize(mul(float4(input.normal.xyz, 0.0), WorldMx).xyz);
    // output.normal = input.normal.xyz;
    output.color = input.color;

    return output;
}

Texture2D shaderTexture;
SamplerState SampleType;

float4 PShader(VOut input) : SV_TARGET
{
    /*const float4 sunDir = {-0.333f, -0.333f, -0.333f, 0.0f};*/
    const float3 sunDir = {0.0f, -1.0f, 0.0f};
    const float4 colorRed = {1.0f, 0.0f, 0.0f, 1.0f};
    const float4 colorBlack = {0.0f, 0.0f, 0.0f, 1.0f};
    
    float cosTheta = dot(sunDir, input.normal);
    if(cosTheta > 0.0f) return colorBlack;
    float4 texColor = shaderTexture.Sample(SampleType, input.worldPos.xz/320.0);
    //return -cosTheta * input.color;
    return -cosTheta * texColor;
    // return float4(input.normal, 1.0f); // return normal as color
}