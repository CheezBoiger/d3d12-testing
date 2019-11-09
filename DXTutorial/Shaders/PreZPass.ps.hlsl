
#include "ShaderGlobalDef.hlsli"

// Global Info, to be sync'ed with cpu GlobalDef.h struct
cbuffer Global : register (b0)
{
    GlobalConstants Global;
};


#ifdef ALPHA_CUTOFF
cbuffer Material : register (b1)
{
    MeshMaterials Material;
};


Texture2D<float4> AlbedoMap : register (t0);
SamplerState SurfaceSampler : register(s0);

#endif

float4 main 
    ( 
#ifdef ALPHA_CUTOFF
        PSInputAlpha Input
#else
        PSInputBasic Input 
#endif
    ) : SV_TARGET
{
#ifdef ALPHA_CUTOFF
    float4 AlbedoColor = AlbedoMap.Sample(SurfaceSampler, Input.TexCoord.xy);
    clip(AlbedoColor.a < 0.5f ? -1 : 1);
#endif
    
    return Global.CameraPos;
}