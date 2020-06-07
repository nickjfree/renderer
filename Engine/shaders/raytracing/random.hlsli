#ifndef __RANDOM__
#define __RANDOM__


// TEA-based pseudo-random number generator
uint RandInit(uint Seed0, uint Seed1)
{
    // Constants cited from "GPU Random Numbers via the Tiny Encryption Algorithm"
    const uint Delta = 0x9e3779b9;
    const uint4 Key = uint4(0xa341316c, 0xc8013ea4, 0xad90777d, 0x7e95761e);
    uint Rounds = 8;

    uint Sum = 0;
    uint2 Value = uint2(Seed0, Seed1);
    for (uint Index = 0; Index < Rounds; ++Index)
    {
        Sum += Delta;
        Value.x += (Value.y + Sum) ^ ((Value.y << 4) + Key.x) ^ ((Value.y >> 5) + Key.y);
        Value.y += (Value.x + Sum) ^ ((Value.x << 4) + Key.z) ^ ((Value.x >> 5) + Key.w);
    }
    
    return Value.x;
}

// Linear congruential generator to evolve pseudo-random numbers
float Rand(inout uint Seed)
{
    // Scale and Bias coefficients are taken from the Park-Miller RNG
    const uint Scale = 48271;
    const uint Bias = 0;
    Seed = Seed * Scale + Bias;

    // Map to significand and divide into [0, 1) range
    float Result = float(Seed & 0x00FFFFFF);
    Result /= float(0x01000000);
    return Result;
}


#endif 