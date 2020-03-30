
#include "ShaderGlobalDef.hlsli"

Texture2D<float4> Normal : register ( t0 );
Texture2D<float> Depth : register ( t1 );

RWStructuredBuffer<float4> ReflectionOut : register ( u0 );

sampler SurfaceSampler : register ( s0 );


cbuffer GlobalConst : register ( b0 )
{
    GlobalConstants Global;
};

cbuffer SSRData : register ( b1 )
{
    SSR Data;
};


float disatanceSquared(float2 a, float2 b)
{
    a -= b;
    return dot(a, a);
}


bool intersectsDepthBuffer(float z, float minZ, float maxZ)
{
    float depthScale = min(1.0f, z * Data.StrideZCutOff);
    z += Data.ZThickness + lerp(0.0f, 2.0f, depthScale);
    return (maxZ >= z) && (minZ - Data.ZThickness <= z);
}


void swap(inout float a, inout float b)
{
    float t = a;
    a = b;
    b = t;
}

float linearizeDepth(float z, float minZ, float maxZ)
{
    return (2 * minZ) / (maxZ + minZ - z * (maxZ - minZ));
}

float linearDepthTexelFetch(int2 hitPixel)
{
    return linearizeDepth(Depth.Load(int3(hitPixel, 0)).r, Global.Near, Global.Far);
}


bool traceScreenSpaceRay
    (
        float3 RayOrigin,
        float3 RayDir,
        float Jitter,
        out float2 HitPixel,
        out float3 HitPoint
    )
{
    return false;
}

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID,
           uint3 GTid : SV_GroupThreadID,
           uint3 Gid: SV_GroupID,
           uint  GI : SV_GroupIndex )
{
}