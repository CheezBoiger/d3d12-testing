//
#include "ShaderGlobalDef.hlsli"
#include "LightingEquations.hlsli"

Texture2D<float4> Albedo : register ( t0 );
Texture2D<float4> Normal : register ( t1 );
Texture2D<float4> RoughnessMetallic : register ( t2 );
Texture2D<float4> ReflectionMask : register ( t3 );
Texture2D<float> Depth : register ( t4 );

cbuffer Global : register ( b0 )
{
    GlobalConstants GConstants;
};

cbuffer Lights : register ( b1 )
{
    LightBuffer LightData;
};

[numthreads(16, 16, 1)]
void main(  uint3 DTid : SV_DispatchThreadID,
            uint3 GTid : SV_GroupThreadID,
            uint GI : SV_GroupIndex )
{
}