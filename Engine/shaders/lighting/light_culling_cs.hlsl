


cbuffer ArraylightInfos: register(b0)
{
	int numLights;
	int lightsPerCell;
	int cellScale;
	int pad;
    float4 lights[256];
}


struct LightIndics
{
    int numLights;
    int lightIndics[15];
};


RWStructuredBuffer<LightIndics> CulledLights : register(u0);


[numthreads(16, 16, 1)]
void CSMain( uint3 groupId : SV_GroupId, uint3 threadId : SV_GroupThreadID)
{
	// cull lights
    CulledLights[threadId.x].numLights = numLights;
    CulledLights[threadId.x].lightIndics[0] = lightsPerCell;
}