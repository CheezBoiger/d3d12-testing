//
#include "ShaderGlobalDef.hlsli"
#include "LightingEquations.hlsli"

Texture2D<float4> AlbedoTarget : register ( t0 );
Texture2D<float4> NormalTarget : register ( t1 );
Texture2D<float4> RoughnessMetallicTarget : register ( t2 );
Texture2D<float4> EmissiveTarget : register ( t3 );

StructuredBuffer<DirectionLight> DirectionLights : register ( t4 );
StructuredBuffer<PointLight> PointLights : register ( t5 );
StructuredBuffer<SpotLight> SpotLights : register ( t6 );

// We use depth to determine position in screenspace, which in turn,
// convert back to world space with the inverse View and Projection matrices.
Texture2D<float> Depth : register ( t4 );

RWTexture2D<float4> OutResult : register ( u0 );

cbuffer GlobalConstant : register ( b0 )
{
    GlobalConstants Global;
};

[numthreads(16, 16, 1)]
void main
    ( 
        uint3 DTid : SV_DispatchThreadID,
        uint3 GTid : SV_GroupThreadID 
    )
{
    float3 PixelColor = float3( 0.0, 0.0, 0.0 );
    float2 UV =  DTid.xy / Global.TargetSize.xy * 2 - 1; // UV in texture space.
    float3 Albedo = AlbedoTarget.Load(DTid).xyz;
    float3 Normal = NormalTarget.Load(DTid).xyz;
    float2 RoughMetal = RoughnessMetallicTarget.Load(DTid).xy;
    float Roughness = RoughMetal.x;
    float Metallic = RoughMetal.y;
    float ZDepth = Depth.Load(DTid).x;
    float4 ClipPos = float4(UV, ZDepth, 1);
    // Calculate the base Fresnel value.
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, Albedo, Metallic);

    // World position without perspective division.
    float4 WorldPosNoPersp = mul(UV, Global.ClipToView);
    // World position with perspective division.
    float3 WorldPos = WorldPosNoPersp / WorldPosNoPersp.w;

    // view vector from user to pixel position.
    float3 V = Global.CameraPos.xyz - WorldPos;

    uint DirectionLightCount = 0;
    uint DirectionLightStride = 0;
    DirectionLights.GetDimensions( DirectionLightCount, DirectionLightStride );

    for ( uint i = 0; i < DirectionLightCount; ++i ) 
    {
        DirectionLight light = DirectionLights[i];
        PixelColor += DirectionLightRadiance(V, Albedo, Normal, Roughness, Metallic, F0, light);
    }

    // Output the final color to the result.
    OutResult[DTid.xy] = float4(PixelColor, 1);
}