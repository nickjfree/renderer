// common effect data
#ifndef  __BRDF__
#define  __BRDF__

/*
    pbr helper functions
*/
#define PI 3.14159265


float3 F_Schlick(in float3 f0, in float f90, in float HdotV)
{
    return f0 + (f90 - f0) * pow(1.f - HdotV, 5.f);
}

float3 FresnelSchlickRoughness(float3 F0, float roughness, float cosTheta)
{
    return F0 + (max(float(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}  

float V_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
    // This is the optimize version 
    float alphaG2 = alphaG * alphaG;
    // Caution: the "NdotL *" and "NdotV *" are explicitely inversed , this is not a mistake. 
    float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
    float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);

    return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}


float DistributionGGX(float3 N, float3 H, float roughness) 
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / (denom + 1e-5f);
}

float GeometrySchlickGGX(float NdotV, float roughness) 
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) 
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

float4 BRDF(float3 N, float3 V, float3 L, float3 f0, float f90, float roughness, float3 albedo, float metallic) {

    float NdotV = abs(dot(N, V)) + 1e-5f;
    // avoid artifact 
    float3 H = normalize(V + L);

    float LdotH = saturate(dot(L, H));
    float NdotH = saturate(dot(N, H));
    float NdotL = saturate(dot(N, L));

    // Specular BRDF
    float  NDF = DistributionGGX(N, H, roughness);        
    float  G   = GeometrySmith(N, V, L, roughness);      
    float3 F   = F_Schlick(f0, f90, saturate(dot(H, V)));

    float3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    float3 specular = numerator / max(denominator, 0.001);

    // Diffuse BRDF 
    float3 kS = F;
    float3 kD = 1 - kS;
    kD = lerp(kD, 0, metallic);
    float3 diffuse = kD * albedo / PI;

    return float4(diffuse + specular, 0);
}

#endif