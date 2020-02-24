//
#include "ShaderGlobalDef.hlsli"
#include "LightingEquations.hlsli"

cbuffer GlobalConstant : register ( b0 )
{
    GlobalConstants Global;
};

Texture2D<float4> AlbedoTarget : register ( t0 );
Texture2D<float4> NormalTarget : register ( t1 );
Texture2D<float4> RoughnessMetallicTarget : register ( t2 );
Texture2D<float4> EmissiveTarget : register ( t3 );

StructuredBuffer<DirectionLight> DirectionLights : register ( t4 );
StructuredBuffer<PointLight> PointLights : register ( t5 );
StructuredBuffer<SpotLight> SpotLights : register ( t6 );
StructuredBuffer<LightTransformation> LightTransforms : register ( t7 );

// We use depth to determine position in screenspace, which in turn,
// convert back to world space with the inverse View and Projection matrices.
Texture2D<float> Depth : register ( t8 );

// Shadow Maps to be indexed depending on the light.
Texture2D<float> SunlightShadowResolve : register ( t9 );
TextureCubeArray<float> PointLightShadowAtlas : register ( t10 );
Texture2DArray<float> SpotLightShadowAtlas : register ( t11 );
Texture2DArray<float> DirectionLightShadowAtlas : register ( t12 );

RWTexture2D<float4> OutResult : register ( u0 );

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
    float4 WorldPosNoPersp = mul(ClipPos, Global.ClipToView);
    // World position with perspective division.
    float3 WorldPos = WorldPosNoPersp / WorldPosNoPersp.w;

    // view vector from user to pixel position.
    float3 V = Global.CameraPos.xyz - WorldPos;

    uint DirectionLightCount = 0;
    uint DirectionLightStride = 0;
    DirectionLights.GetDimensions( DirectionLightCount, DirectionLightStride );

    for ( uint i = 0; i < DirectionLightCount; ++i ) 
    {
        DirectionLight Light = DirectionLights[i]; 
        float3 Radiance = DirectionLightRadiance(V, Albedo, Normal, Roughness, Metallic, F0, Light);
        // Look for the direction light that contains the sunlight shadow.
        if (Global.SunLightShadowIndex == i) {
            LightTransformation LightTransform = LightTransforms[Light.LightTransformIndex];
            float2 ShadowCoord = CalculateShadowCoord(WorldPos, LightTransform.ViewToClip);
            Radiance *= DirectionLightShadowAtlas.Load(int4(ShadowCoord, i, 0)).r;
        }
        PixelColor += Radiance; 
    }

    // Output the final color to the result.
    OutResult[DTid.xy] = float4(PixelColor, 1);
}