
#include "../common/deferred.h"
#include "../common/basic_layout.h"
#include "../lighting/lighting.h"
#include "../common/post.h"


Texture2D gRtTarget : register(t0); 
/*
    reflection resolve pixel shader
*/

PS_Output_Simple PS(PS_Input_Simple input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;
    float2 uv = input.TexCoord;

    float4 Color = gPostBuffer.Sample(gSam, input.TexCoord);

    GBuffer gbuffer = GetGBuffer(uv);

    float roughness = gbuffer.Roughness;
    float3 N = gbuffer.Normal.xyz;
    float3 V = gbuffer.View;
    float3 specularColor = gbuffer.Specular;
    float NoV = dot(N, V);

    float3 reflection = gRtTarget.Sample(gSam, input.TexCoord).xyz;
    // pre-intergrated texture
    reflection =  reflection * EnvBRDF(specularColor, roughness, NoV);

    output.Color = Color + 3* float4(reflection, 0);

    //output.Color = float4(reflection, 0);

    // output.Color = gMotionVector.Sample(gSam, input.TexCoord).w/2048.0f;
    return output;
}
