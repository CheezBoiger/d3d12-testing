// Definitions of shader structs and layouts.
// May contain other definitions as well...
#ifndef SHADER_GLOBAL_DEF_H
#define SHADER_GLOBAL_DEF_H

// Global constants buffer.
struct GlobalConstants 
{
    float4 CameraPos;
    float4x4 ViewToWorld;
    float4x4 WorldToView;
    float4x4 Proj;
    float4x4 ViewToClip;
    float4x4 ClipToView;
    uint4 TargetSize;
    int AllowBumpMapping;
    int3 Pad0;
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
    float4 RoughnessMetallicFactor;
    float4 EmissionFactor;
    float4 AlbedoFactor;
    float4 FresnelFactor;
};


struct PSInputGeometry
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL;
    float4 Tangent : TANGENT0; 
    float4 BiTangent : BINORMAL0;
    float4 TexCoords : TEXCOORD0;
    float4 ScreenPosition : TEXCOORD1;
    float4 WorldPosition : TEXCOORD2;
};


struct VSInputGeometry
{
    float4 Position : POSITION;
    float4 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float4 TexCoord : TEXCOORD0;
};


struct VSDepthInputGeometry
{
    float4 Position : POSITION;
};


struct PSInputBasic
{
    float4 Position : SV_POSITION;
};


struct PSInputAlpha
{
    float4 Position : SV_POSITION;
    float4 TexCoord : TEXCOORD;
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


float3 calculateBitangent(float3 N, float3 T)
{
    return normalize(cross(N, T));
}

#endif // SHADER_GLOBAL_DEF_H