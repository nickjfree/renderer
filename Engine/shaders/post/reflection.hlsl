
#include "../common/deferred.h"
#include "../common/basic_layout.h"

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
    float4 Reflection = gRtTarget.Sample(gSam, input.TexCoord);

    output.Color = Color + 0.3 * Reflection;

    // output.Color = Reflection;

    return output;
}
