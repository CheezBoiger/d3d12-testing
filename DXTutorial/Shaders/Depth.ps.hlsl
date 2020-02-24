// Depth Pixel shader.
#include "CommonShaderParams.h"
#include "ShaderGlobalDef.hlsli"

Texture2D<float4> AlbedoSurface : register ( t0 );
sampler SurfaceSampler : register ( s0 );

float4 main( PSInputAlpha Input ) : SV_TARGET
{
	float4 Alpha = AlbedoSurface.Sample(SurfaceSampler, Input.TexCoord).a;
	clip(Alpha < 0.5 ? -1 : 1);
	return float4(0, 0, 0, 1);
}