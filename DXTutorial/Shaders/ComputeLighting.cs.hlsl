//
#include "ShaderGlobalDef.hlsli"
#include "LightingEquations.hlsli"

Texture2D<float4> Albedo : register ( t0 );
Texture2D<float4> Normal : register ( t1 );
Texture2D<float4> RoughnessMetallic : register ( t2 );
Texture2D<float4> Emissive : register ( t3 );

// We use depth to determine position in screenspace, which in turn,
// convert back to world space.
Texture2D<float> Depth : register ( t4 );

cbuffer GlobalConstant : register ( b0 )
{
    GlobalConstants Global;
};

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}