// Shader Code for a pre z pass.
#include "ShaderGlobalDef.hlsli"

// PerMesh constant buffer, to be sync'ed with cpu GlobalDef.h struct.
cbuffer PerMesh : register (b1)
{
    MeshTransforms Mesh;
};


#ifdef ALPHA_CUTOFF
    PSInputAlpha
#else
    PSInputBasic
#endif
    VSMain 
    (
        VSInputGeometry VertexInput
    )
{
#ifdef ALPHA_CUTOFF
    PSInputAlpha Input;
    Input.TexCoord = VertexInput.TexCoord;
#else
    PSInputBasic Input;
#endif

  VertexInput.Position.w = 1.0f;
  Input.Position = mul(Mesh.WorldToViewClip, VertexInput.Position);



  return Input;
}