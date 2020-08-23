
#include "../common/deferred.h"
#include "../common/basic_layout.h"
#include "../common/basic_registers.h"

#include "../common/post.h"



//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format

// implement a ssao effect
//parameters
static const float random_size = 64;
static const float2 vec[4] = {float2(1,0),float2(-1,0),float2(0,1),float2(0,-1)};
static const float g_scale = 0.01f;
static const float g_bias = 0.2;
static const float g_intensity = 2;


// sample random noise texture
float2 GetRandom(float2 uv)
{
    float4 Noise = gDiffuseMap0.Sample(gSam, frac(uv * gScreenSize.xy / random_size));
    Noise = Noise * 2 - 1;
    Noise = normalize(Noise);
    return Noise.xy;
}


// calc ao value
float CalcAO(float2 coord, float3 p0, float3 n)
{
    float3 p1 = GetPosition(coord);
    float3 distance = p1 - p0;
    float3 v = normalize(distance);
    float d = length(distance) * g_scale;
    return max(0, dot(n,v) - g_bias) * (1.0f / (1.0f + d)) * g_intensity;
//  return p1;
}

/*
    ssao pixel shader
*/
PS_Output_Simple PS(PS_Input_Simple input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;
    float2 uv = input.TexCoord;

    float4 Color = gPostBuffer.Sample(gSam, input.TexCoord);
    float3 Normal = GetNormal(uv).xyz;
    float3 Position = GetPosition(uv);

    if (length(Position) < 0.001) {
        output.Color = Color;
        return output;
    }

    float2 Noise = GetRandom(uv);
    float ao = 0;
    float radius = 0.1f;
    radius = radius/Position.z;
    int iterations = 4;

    [unroll(4)]
    for(int i = 0; i < iterations; i++)
    {
        float2 coord1 = reflect(vec[i], Noise) * radius;
        //float2 coord1 = vec[i] *  radius;
        float2 coord2 = float2(coord1.x*0.707 - coord1.y*0.707,coord1.x*0.707 + coord1.y*0.707);
        ao += CalcAO(uv + coord1 * 0.25, Position, Normal);
        ao += CalcAO(uv + coord2 * 0.5, Position, Normal);
        ao += CalcAO(uv + coord1 * 0.75, Position, Normal);
        ao += CalcAO(uv + coord2, Position, Normal);
    }
    ao /= float(iterations) * 4;
    ao = saturate(ao);
    output.Color = lerp(Color, 0.01 * Color, ao);

    if(isnan(output.Color.x)) 
    {
        output.Color = Color;
    }
    
    return output;
}
