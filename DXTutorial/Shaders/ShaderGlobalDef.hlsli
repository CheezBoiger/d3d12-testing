// Definitions of shader structs and layouts.
// May contain other definitions as well...
#ifndef SHADER_GLOBAL_DEF_H
#define SHADER_GLOBAL_DEF_H

#include "CommonShaderParams.h"


// Global constants buffer.
struct GlobalConstants 
{
    float4 CameraPos;
    float4x4 ViewToWorld;
    float4x4 WorldToView;
    float4x4 Clip;
    float4x4 ViewToClip;
    float4x4 ClipToView;
    uint4 TargetSize;
    int AllowBumpMapping;
    float Near;
    float Far;
    int SunLightShadowIndex; // Index of directional light that uses the sunlight shadow resolve. -1 if not corresponding to one.
};


struct SSR
{
    uint2 TargetSize;
    float ZThickness;
    float NearZ;
    float Stride;
    float MaxSteps;
    float MaxDistance;
    float StrideZCutOff;
    float NumMips;
    float FadeStart;
    float FadeEnd;
    float Pad0;
};

// Mesh Transform data.
struct MeshTransforms 
{
    float4x4 World;
    float4x4 WorldToViewClip;
    float4x4 PrevWorldToViewClip;
    float4x4 N;
};

// Per material struct.
struct MeshMaterials 
{
    // Roughness = x, Metallic = y;
    float4 Albedo;
    float4 RoughnessMetallicFactor;
    float4 EmissionFactor;
    float4 AlbedoFactor;
    float4 FresnelFactor;
    uint4 MaterialFlags;
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
    float4 Position : SV_POSITION;
};


struct PSInputVelocity
{
    float4 Position : SV_POSITION;
    float4 ClipPosition : POSITION0;
    float4 PrevClipPosition : POSITION1;
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


struct PSVelocityOutput
{
    float4 Velocity : SV_TARGET0;
};


struct PSInputUVOnly
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};


float3 CalculateBinormal(float3 N, float3 T)
{
    return normalize(cross(N, T));
}


// If using a more traditional pipeline, it would be best to calculate it's metallic mask.
float SolveForMetallic(float3 Diffuse, float Specular, float OneMinusSpecularStrength)
{
    const float3 DielectricSpec = float3(   DIELECTRIC_SPECULAR_VALUE, 
                                            DIELECTRIC_SPECULAR_VALUE, 
                                            DIELECTRIC_SPECULAR_VALUE);
    if (Specular < DielectricSpec.r)
        return 0;

    float A = DielectricSpec.r;
    float B = Diffuse * OneMinusSpecularStrength / (1 - DielectricSpec.r) + Specular - 2 * DielectricSpec.r;
    float C = DielectricSpec.r - Specular;
    float D = max(B * B - 4 * A * C, 0);
    return clamp((-B + sqrt(D)) / (2 * A), 0, 1);
}

// Calculating roughness from a Specular/Gloss pipeline, we simply need the inverse of the glossiness map.
float SolveForRoughness(float glossiness)
{
    return 1 - glossiness;
}
#endif // SHADER_GLOBAL_DEF_H