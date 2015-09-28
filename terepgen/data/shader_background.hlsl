
cbuffer SceneBuffer : register(b0)
{
	float4x4 ViewProjMx;
	float4x4 ViewMx;
};
cbuffer ObjectBuffer : register(b1)
{
	float4x4 WorldMx;
};

SamplerState SampleType;
Texture2D skyTexture : register(t2);

struct SkyVIn
{
    float3 position : POSITION;
};

struct SkyVOut
{
    float4 screenPos : SV_POSITION;
    float3 worldPos : POSITION;
};

SkyVOut BackgroundVShader(SkyVIn input)
{
    SkyVOut output;
    output.screenPos = float4(input.position, 1);
    output.worldPos = input.position;
    
    return output;
}

float4 BackgroundPShader(SkyVOut input) : SV_TARGET
{
    float2 texPos = (input.worldPos.xy + float2(1.0, -1.0)) / float2(2.0, 2.0); 
    float4 tex = skyTexture.Sample(SampleType, texPos);
    return tex;
}