// Definitions of shader structs and layouts.
// May contain other definitions as well...
#ifndef SHADER_GLOBAL_DEF_H
#define SHADER_GLOBAL_DEF_H

// Global constants buffer.
struct GlobalConstants 
{
    float4 vCameraPos;
    float4x4 mViewToWorld;
    float4x4 mWorldToView;
    float4x4 mProj;
    float4x4 mViewToClip;
    float4x4 mClipToView;
    uint4 uTargetSize;
};

// Mesh Transform data.
struct MeshTransforms 
{
    float4x4 World;
    float4x4 WorldToViewClip;
    float4x4 PrevWorldToViewClip;
    float4x4 N;
    float4 MaterialFlags;
};

// Per material struct.
struct MeshMaterials 
{
    // Roughness = x, Metallic = y;
    float4 RoughnessMetallic;
    float4 Emission;
    float4 AlbedoFactor;
    float4 FresnelFactor;
};


struct PSInputGeometry
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL;
    float4 TexCoords : TEXCOORD0;
    float4 ScreenPosition : TEXCOORD1;
    float4 WorldPosition : TEXCOORD2;
};


struct PSInputBasic
{
    float4 Position : SV_POSITION;
};


struct PSOutputGBuffer
{
    float4 Albedo : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 RoughnessMetallic : SV_TARGET2;
    float4 Emission : SV_TARGET3;
};


struct PSOutputVelocity
{
    float4 Velocity : SV_TARGET0;
};

#endif // SHADER_GLOBAL_DEF_H