// Geometry pass.
#include "ShaderGlobalDef.hlsli"


cbuffer Global : register (b0)
{
    GlobalConstants Globals;
};


cbuffer PerMaterial : register (b2)
{
    MeshMaterials Material;
};

Texture2D<float4> AlbedoMap : register (t0);
Texture2D<float4> NormalMap : register (t1);
Texture2D<float4> RoughnessMetallicMap : register (t2);
Texture2D<float4> EmissionMap : register (t3);

SamplerState SurfaceSampler : register (s0);


PSOutputGBuffer main ( PSInputGeometry Input ) 
{
    PSOutputGBuffer GBuffer;
    float3 Tangent = Input.Tangent.xyz;
    float3 BiTangent = Input.BiTangent.xyz;
    float3 Normal = Input.Normal.xyz;

    float3 AlbedoColor = Material.Color;
    float3 NormalColor = Normal;

    if ( Material.MaterialFlags.x & MATERIAL_USE_ALBEDO_MAP ) { 
        AlbedoColor = AlbedoMap.Sample( SurfaceSampler, Input.TexCoords.xy ).rgb;
    }

    if ( Material.MaterialFlags.x & MATERIAL_USE_NORMAL_MAP ) {
        NormalColor = NormalMap.Sample( SurfaceSampler, Input.TexCoords.xy ).rgb;
    }

    float4 RoughMetalColor = float4( Material.RoughnessMetallicFactor.xy, 0, 0 );

    if ( Material.MaterialFlags.x & ( MATERIAL_USE_METALLIC | MATERIAL_USE_ROUGHNESS ) ) {
        RoughMetalColor = RoughnessMetallicMap.Sample( SurfaceSampler, Input.TexCoords.xy );
    }

    float4 EmissionColor = EmissionMap.Sample( SurfaceSampler, Input.TexCoords.xy );

    // Set to [0.0 - 1.0] normal.
    NormalColor = NormalColor * 2.0 - 1.0;

    // Allow bump mapping if needed. Must calculate the tangent space matrix and apply to the 
    // normal.
    if ( Globals.AllowBumpMapping >= 1 ) {
        float3x3 TangentSpaceMatrix = float3x3 ( Tangent, BiTangent, Normal );
        float3 BumpNormal = mul( TangentSpaceMatrix, NormalColor );
        NormalColor = BumpNormal;
    }

    GBuffer.Albedo = float4( AlbedoColor * Material.AlbedoFactor.xyz , 1.0 );
    GBuffer.Normal = float4( NormalColor, 1.0 );
    GBuffer.RoughnessMetallic = float4( RoughMetalColor.xy, 0.0, 1.0 );
    GBuffer.Emission = float4( EmissionColor.xyz, Material.EmissionFactor.x );

    return GBuffer;
}