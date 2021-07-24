
#include "../common/deferred.h"
#include "../common/basic_layout.h"
#include "../common/basic_registers.h"

#include "../common/post.h"


/*
    ssao pixel shader
*/
PS_Output_Simple PS(PS_Input_Simple input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;
    // sample gbuffer
    GBuffer gbuffer = GetGBuffer(input.TexCoord);

    if (length(gbuffer.Position) < 0.001) {
        output.Color = float4(0, 0, 0, 0);
        return output;
    }
    // get diffuse
    float4 diffuse = gbuffer.Diffuse.xyzw;
    // fake lighting    
    output.Color = float4(1, 0, 0, 0) * 10.0f * (1 - diffuse.a);
    return output;
}
