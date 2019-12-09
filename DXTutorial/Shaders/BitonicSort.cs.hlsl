//

#define BITONIC_SORT_THREADS 256

RWStructuredBuffer<uint> Data : register( u0 ); 

cbuffer WidthInfo : register ( b0 )
{
    uint g_Level;
    uint g_LevelMask;  
    uint pad0;
    uint pad1;
};


groupshared uint sharedData[BITONIC_SORT_THREADS];

[numthreads(BITONIC_SORT_THREADS, 1, 1)]
void main
    ( 
        uint3 DTid : SV_DispatchThreadID,
        uint3 GTid : SV_GroupThreadID,
        uint3 Gid : SV_GroupID,
        uint GI : SV_GroupIndex 
    )
{
    sharedData[ GI ] = Data[ DTid.x ];
    GroupMemoryBarrierWithGroupSync();

    for (uint i = g_Level >> 1; i > 0; i >>= 1)
    {
        uint result = (( sharedData[ GI & ~i ] <= sharedData[ GI | i ]) == (bool)(g_LevelMask & DTid.x)) ? sharedData[ GI ^ i ] : sharedData[ GI ];
        GroupMemoryBarrierWithGroupSync();
    
        sharedData[ GI ] = result;
        GroupMemoryBarrierWithGroupSync();
    }

    Data[ DTid.x ] = sharedData[ GI ];
}