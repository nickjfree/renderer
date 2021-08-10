

#include "common.h"
#include "deferred.h"
/*
    vertex shader for none instancing rendering
*/
PS_Input_GBuffer VS_Basic_GBuffer(VS_Input_Simple vs_input) 
{
    PS_Input_GBuffer output;
    output = transform_to_view_gbuffer(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, 
        gWorldViewProjection, gWorldViewMatrix, gPrevWorldViewProjection,
        gObjectId);
    return output;
}

/*
    vertex shader for none instancing rendering
*/
PS_Input_Simple VS_Basic_Simple(VS_Input_Simple vs_input) 
{
    PS_Input_Simple output;
    output = transform_to_view_simple(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, gWorldViewProjection, gWorldViewMatrix);
    return output;
}

/*
    vertex shader for instancing rendering
*/
PS_Input_GBuffer VS_Instancing_GBuffer(VS_Input_Instance vs_input) 
{
    PS_Input_GBuffer output;
    output = transform_to_view_gbuffer(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, 
        vs_input.InstanceWVP, vs_input.InstanceWV, vs_input.InstancePWVP,
        vs_input.InstanceObjectId);
    return output;
}

/*
    vertex shader for instancing rendering 
*/
PS_Input_Simple VS_Instancing_Simple(VS_Input_Instance vs_input) 
{
    PS_Input_Simple output;
    output = transform_to_view_simple(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, vs_input.InstanceWVP, vs_input.InstanceWV);
    return output;
}

/*
    gbuffer pixel shader
*/
PS_Output_GBuffer PS_GBuffer(PS_Input_GBuffer ps_input)
{   
    PS_Output_GBuffer output = (PS_Output_GBuffer)0;
    // read gbuffers
    float4 normal = gNormalMap0.Sample(gSam, ps_input.TexCoord);
    float4 diffuse = gDiffuseMap0.Sample(gSam, ps_input.TexCoord);
    float4 specular = gSpecularMap0.Sample(gSam, ps_input.TexCoord);
    // normalize normal vector
    normal = normal * 2.0 - 1;
    normal.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y);
    normal.w = 0;
    normal = normalize(normal);
    normal = ps_input.Normal * normal.z + normal.x * ps_input.Tangent + normal.y * ps_input.BiNormal;
    normal = normalize(normal);
    // output
    output.Compact.xy = EncodeNormal(normal.xyz);
    // depth: view space z value
    output.Depth.x = ps_input.Depth;
    output.Diffuse = diffuse;
    // x: specular  y: roughness  z: metallic
    output.Specular = float4(gSpecular, specular.y, specular.z, 0);
    // compact info: object id
    output.Compact.w = ps_input.ObjectId;

    // output.Specular = float4(gSpecular, specular.y, 1, 0);
    // motion vectors
    float2 currentScreen = ps_input.CurrentPosH.xy / ps_input.CurrentPosH.w * 0.5 + 0.5;
    float2 prevScreen = ps_input.PrevPosH.xy / ps_input.PrevPosH.w * 0.5 + 0.5;   
    float  linearZ = ps_input.CurrentPosH.z;
    currentScreen.y = 1 - currentScreen.y;
    prevScreen.y = 1 - prevScreen.y;
    // motion vector valid or not
    float valid = reprojectionValid(prevScreen, ps_input.ObjectId, normal.xyz, linearZ);
    // z grad
    float fwidthZ = max(abs(ddx(linearZ)), abs(ddy(linearZ)));
    //set motion vector and compact buffer,  xy: motion, z: fwidthZ, w: isValid
    output.Motion = float4(prevScreen.xy - currentScreen.xy, fwidthZ, valid);
    // history length
    output.Compact.z = linearZ;
    return output;
}


/*
    simple pixel shader (for shadow mapping)
*/

PS_Output_Simple PS_Simple(PS_Input_Simple ps_input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;
    output.Color = gDiffuseMap0.Sample(gSam, ps_input.TexCoord);
    // output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}