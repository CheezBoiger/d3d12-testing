// Geometry pass.
#include "ShaderGlobalDef.h"

cbuffer PerMaterial : register (b2)
{
    MeshMaterials Material;
};

PSOutputGBuffer main ( PSInputGeometry input ) 
{
    PSOutputGBuffer gbuffer;
    gbuffer.Albedo = float4(0.0, 0.0, 0.0, 1.0);
    gbuffer.Normal = float4(0.0, 0.0, 0.0, 1.0f);
    gbuffer.RoughnessMetallic = float4(0.0, 0.0, 0.0, 1.0f);
    gbuffer.Emission = float4(0.0, 0.0, 0.0, 1.0f);
	return gbuffer;
}