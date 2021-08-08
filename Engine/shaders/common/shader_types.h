#ifndef __SHADER_TYPES__
#define __SHADER_TYPES__


#ifdef __cplusplus // c++ side


#define CB_SLOT(name) __CBSLOT__##name##__
#define CBUFFER(name, slot) constexpr int CB_SLOT(name) = slot; struct alignas(16) name


#else   // hlsl side

#define CBUFFER(name, slot) cbuffer name : register(b ## slot)
#define SAMPLERSTATE(name, slot) SamplerState name : register(s ## slot)


#endif // __cplusplus



#endif // __SHADER_TYPES__