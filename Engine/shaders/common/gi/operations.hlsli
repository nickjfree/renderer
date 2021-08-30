#ifndef __OPERATIONS__
#define __OPERATIONS__

#define PI 3.14159265

/*
* Computes a low discrepancy spherically distributed direction on the unit sphere,
* for the given index in a set of samples. Each direction is unique in 
* the set, but the set of directions is always the same.
*/
float3 SphericalFibonacci(float index, float numSamples)
{
    const float b = (sqrt(5.f) * 0.5f + 0.5f) - 1.f;
    float phi = 2.f * PI * frac(index * b);
    float cosTheta = 1.f - (2.f * index + 1.f) * (1.f / numSamples);
    float sinTheta = sqrt(saturate(1.f - (cosTheta * cosTheta)));

    return float3((cos(phi) * sinTheta), (sin(phi) * sinTheta), cosTheta);
}


#endif