#include "../common/common.h"


Texture2D gInput : register(t0);
Texture2D gOutput : register(t1);

[numthreads(64, 1, 1)]
void CSMain(uint3 groupId : SV_GroupId, uint3 threadId : SV_GroupThreadID)
{

}

/*
    simple pixel shader just copy texture
*/
PS_Output_Simple PS(PS_Input_Simple ps_input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;
    output.Color = gInput.Sample(gSam, ps_input.TexCoord);
    return output;
}
