#include "ShaderGlobalDef.hlsli"

cbuffer PerMesh : register (b1)
{
    MeshTransforms Mesh;
};

PSInputVelocity main( VSInputGeometry Input )
{
    PSInputVelocity Output;

    Output.ClipPosition = mul(Mesh.WorldToViewClip, Input.Position);
    Output.PrevClipPosition = mul(Mesh.PrevWorldToViewClip, Input.Position);
    
    Output.Position = Output.ClipPosition;
	return Output;
}