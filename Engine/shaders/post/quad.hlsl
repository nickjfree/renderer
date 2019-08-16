/*
    screen space quad
*/

#include "../common/basic_layout.h"


// render a screen space quad
PS_Input_Simple VS(VS_Input_Simple input)
{
    // simple ps shader input layout
    PS_Input output = (PS_Input_Simple)0;
    // caculate screen space
    output.PosH.xy = 2 * input.PosL.xy - 1;
    output.PosH.zw = 1;
    output.TexCoord = input.TexCoord;
    return output;
}