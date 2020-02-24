//
#include "CommonShaderParams.h"
#include "ShaderGlobalDef.hlsli"

cbuffer MeshTransform : register ( b0 )
{
	MeshTransforms Mesh;
};

PSInputAlpha main( VSInputGeometry Input )
{
	PSInputAlpha Output;
	Output.Position = float4( mul( Input.Position, Mesh.WorldToViewClip ) );
	Output.TexCoord = Input.TexCoord;
	return Output;
}