//
#include "CommonShaderParams.h"
#include "ShaderGlobalDef.hlsli"

cbuffer MeshTransform : register ( b0 )
{
	MeshTransforms Mesh;
};

cbuffer PerspectiveTransform : register ( b1 )
{
	float4x4 ViewToClip;
};


PSInputBasic main( VSInputGeometry Input )
{
	PSInputBasic Output;
	float4x4 WorldToViewClip = mul( ViewToClip, Mesh.World );
	Output.Position = mul( WorldToViewClip, Input.Position );
	//Output.TexCoord = Input.TexCoord;
	return Output;
}