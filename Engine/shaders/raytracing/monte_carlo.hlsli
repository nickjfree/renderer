#ifndef __MONTE_CARLO__
#define __MONTE_CARLO__

#define PI 3.14159265


float Pow2(float x) {
    return x*x;
}


float Pow4(float x) {
    return x*x*x*x;
}


float3x3 GetTangentBasis( float3 TangentZ )
{
    const float Sign = TangentZ.z >= 0 ? 1 : -1;
    const float a = -rcp( Sign + TangentZ.z );
    const float b = TangentZ.x * TangentZ.y * a;
    
    float3 TangentX = { 1 + Sign * a * Pow2( TangentZ.x ), Sign * b, -Sign * TangentZ.x };
    float3 TangentY = { b,  Sign + a * Pow2( TangentZ.y ), -TangentZ.y };

    return float3x3( TangentX, TangentY, TangentZ );
}


float2 UniformSampleDisk( float2 E )
{
    float Theta = 2 * PI * E.x;
    float Radius = sqrt( E.y );
    return Radius * float2( cos( Theta ), sin( Theta ) );
}


float4 ImportanceSampleVisibleGGX( float2 DiskE, float a2, float3 V )
{
    // TODO float2 alpha for anisotropic
    float a = sqrt(a2);

    // stretch
    float3 Vh = normalize( float3( a * V.xy, V.z ) );

    // Orthonormal basis
    // Tangent0 is orthogonal to N.
    float3 Tangent0 = (Vh.z < 0.9999) ? normalize( cross( float3(0, 0, 1), Vh ) ) : float3(1, 0, 0);
    float3 Tangent1 = cross( Vh, Tangent0 );

    float2 p = DiskE;
    float s = 0.5 + 0.5 * Vh.z;
    p.y = (1 - s) * sqrt( 1 - p.x * p.x ) + s * p.y;

    float3 H;
    H  = p.x * Tangent0;
    H += p.y * Tangent1;
    H += sqrt( saturate( 1 - dot( p, p ) ) ) * Vh;

    // unstretch
    H = normalize( float3( a * H.xy, max(0.0, H.z) ) );

    float NoV = V.z;
    float NoH = H.z;
    float VoH = dot(V, H);

    float d = (NoH * a2 - NoH) * NoH + 1;
    float D = a2 / (PI*d*d);

    float G_SmithV = 2 * NoV / (NoV + sqrt(NoV * (NoV - NoV * a2) + a2));

    float PDF = G_SmithV * VoH * D / saturate(NoV);
    
    return float4(H, PDF);
}


float4 GenerateReflectedRayDirection(
    float3 IncidentDirection,
    float3 WorldNormal,
    float Roughness,
    float2 RandSample
)
{
    float3 RayDirection;
    float invPDF;
    if (Roughness < 0.001) //ReflectionSmoothClamp)
    {
        RayDirection = reflect(IncidentDirection, WorldNormal);
        invPDF = 1;
    }
    else
    {
        float3 N = WorldNormal;
        float3 V = -IncidentDirection;
        float2 E = RandSample;

        float3x3 TangentBasis = GetTangentBasis(N);
        float3 TangentV = mul(TangentBasis, V);

        float NoV = saturate(dot(V, WorldNormal));

        float4 Sample = ImportanceSampleVisibleGGX(UniformSampleDisk(E), Pow4(Roughness), TangentV);

        float3 H = mul(Sample.xyz, TangentBasis);
        float3 L = 2 * dot(V, H) * H - V;

        RayDirection = L;
        invPDF = 1.0/Sample.w;
    }
    return float4(RayDirection, invPDF);
}

void FixSampleDirectionIfNeeded(float3 SmoothSurfaceNormal, inout float3 SampleDirection)
{
    if (dot(SmoothSurfaceNormal, SampleDirection) < 0.0)
    {
        // The sampling direction is pointing towards the surface, so revert it along the normal axis.
        SampleDirection = SampleDirection - 2.0f * dot(SmoothSurfaceNormal, SampleDirection) * SmoothSurfaceNormal;
    }
}

#endif
