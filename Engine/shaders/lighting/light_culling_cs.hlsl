


struct BufType
{
    int i;
    float f;
};


RWStructuredBuffer<BufType> BufferOut : register(u0);


[numthreads(16, 16, 1)]
void CSMain( uint3 groupId : SV_GroupId, uint3 threadId : SV_GroupThreadID)
{
    BufferOut[threadId.x].i = 100;
    BufferOut[threadId.x].f = 101;

}