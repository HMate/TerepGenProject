
cbuffer SceneBuffer : register(b0)
{
    float4x4 ViewProjMx;
    float4x4 ViewMx;
};
cbuffer ObjectBuffer : register(b1)
{
    float4x4 WorldMx;
    float4 cameraDir;
};

SamplerState SampleType;
TextureCube skyTexture : register(t2);

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
    float4x4 TVM = transpose(ViewMx);
    output.worldPos = mul(float4(input.position, 1.0), TVM);
    // output.worldPos = float3(input.position);
    
    return output;
}

float4 BackgroundPShader(SkyVOut input) : SV_TARGET
{
    // float3 texPos = (input.screenPos.xyz + float3(1.0, -1.0, 1.0)) / float3(2.0, 2.0, 2.0); 
    float3 skyPos = normalize(input.worldPos);
    float3 texPos = normalize(float3(skyPos.x, cameraDir.y + (skyPos.y * 1.33), skyPos.z));
    // float3 texPos = normalize(cameraDir + skyPos);
    float4 tex = skyTexture.Sample(SampleType, texPos);
    return tex;
}