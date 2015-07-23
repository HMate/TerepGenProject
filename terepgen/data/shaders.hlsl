
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
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 color : COLOR;
};

struct VOut
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float4 worldPos : POSITION;
};

VOut VShader(VIn input)
{
    VOut output;

    output.worldPos = mul(input.position, WorldMx);
    output.position = mul(output.worldPos, ViewProjMx);
    //output.position = input.position;
    
    // NOTE: normal transformation needs the transpose of the inverse of WorldMX
    // HLSL dont have inverse, but if WorldMX is orthogonal, we dont need it.
    // just use the worldmx and normalize the vector.
    // WorldMX is orthogonal, if scaling is uniform.
    // SOURCE: http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
    //output.normal = normalize(input.normal.xyz);
    output.normal = normalize(mul(input.normal.xyz, WorldMx));
    output.color = input.color;

    return output;
}
  
float4 PShader(VOut input) : SV_TARGET
{
    /*const float4 sunDir = {-0.333f, -0.333f, -0.333f, 0.0f};*/
    const float3 sunDir = {0.0f, -1.0f, 0.0f};
    const float4 colorRed = {1.0f, 0.0f, 0.0f, 1.0f};
    const float4 colorBlack = {0.0f, 0.0f, 0.0f, 1.0f};
    
    float cosTheta = dot(sunDir, input.normal);
    if(cosTheta > 0.0f) return colorBlack;
    return -cosTheta * input.color;
    return float4(input.normal, 1.0f); // return normal as color
}