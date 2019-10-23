// Geometry pass.
#include "ShaderGlobalDef.hlsli"


cbuffer Global : register (b0)
{
    GlobalConstants Globals;
};


cbuffer PerMesh : register (b1)
{
    MeshTransforms Mesh;
};


cbuffer PerMaterial : register (b2)
{
    MeshMaterials Material;
};


PSOutputGBuffer main ( PSInputGeometry Input ) 
{
    PSOutputGBuffer gbuffer; 

    gbuffer.Albedo = float4(0.0, 0.0, 0.0, 1.0);
    gbuffer.Normal = float4(0.0, 0.0, 0.0, 1.0f);
    gbuffer.RoughnessMetallic = float4(0.0, 0.0, 0.0, 1.0f);
    gbuffer.Emission = float4(0.0, 0.0, 0.0, 1.0f);

    return gbuffer;
}