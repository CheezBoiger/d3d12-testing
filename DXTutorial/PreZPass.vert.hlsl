// Shader Code for a pre z pass.
#include "ShaderGlobalDef.h"

// PerMesh constant buffer, to be sync'ed with cpu GlobalDef.h struct.
cbuffer PerMesh : register (b1)
{
    MeshTransforms Mesh;
};


PSInputBasic VSMain ( float4 position : POSITION, 
                float4 normal : NORMAL,
                float4 texCoord : TEXCOORD )
{
  PSInputBasic input;
  position.w = 1.0f;
  input.Position = mul(Mesh.WorldToViewClip, position);
  return input;
}