
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

Texture2D grassTexture : register(t0);
Texture2D rockTexture : register(t1);
SamplerState SampleType;

float4 PShader(VOut input) : SV_TARGET
{
    /*const float4 sunDir = {-0.333f, -0.333f, -0.333f, 0.0f};*/
    const float3 sunDir = {0.0f, -1.0f, 0.0f};
    const float4 colorRed = {1.0f, 0.0f, 0.0f, 1.0f};
    const float4 colorBlack = {0.0f, 0.0f, 0.0f, 1.0f};
    // const float3 tex_scale = {1.0/450.0, 1.0/160.0, 1.0/325.0};
    const float3 tex_scale = {0.1, 0.1, 0.1};
    
    float cosTheta = dot(sunDir, input.normal);
    if(cosTheta > 0.0f) cosTheta = -cosTheta/3.0;
    
    //NOTE: Texture blending based on http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
    
    float3 blend_weights = abs(input.normal.xyz);
    blend_weights = (blend_weights - 0.25) * 7;  
    blend_weights = max(blend_weights, 0);      // Force weights to sum to 1.0 (very important!)  
    blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z ).xxx;   
    // Now determine a color value and bump vector for each of the 3  
    // projections, blend them, and store blended results in these two  
    // vectors:  
    float4 blended_color;  
    {  
        // Compute the UV coords for each of the 3 planar projections.  
        // tex_scale (default ~ 1.0) determines how big the textures appear.  
        float2 coord1 = input.worldPos.yz * tex_scale.x;
        float2 coord2 = input.worldPos.xz * tex_scale.y;
        float2 coord3 = input.worldPos.xy * tex_scale.z;
        // Sample color maps for each projection, at those UV coords.  
        float4 col1 = rockTexture.Sample(SampleType, coord1);  
        float4 col2;
        if(input.normal.y > 0.0)
            col2 = grassTexture.Sample(SampleType, coord2);  
        else
            col2 = rockTexture.Sample(SampleType, coord2);
        float4 col3 = rockTexture.Sample(SampleType, coord3);   
        // Finally, blend the results of the 3 planar projections.  
        blended_color = col1.xyzw * blend_weights.xxxx +  
                        col2.xyzw * blend_weights.yyyy +  
                        col3.xyzw * blend_weights.zzzz;   
    }
    return -cosTheta * blended_color;
    // return float4(input.normal, 1.0f); // return normal as color
}