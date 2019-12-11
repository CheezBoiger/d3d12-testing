
#include "ShaderGlobalDef.hlsli"

Texture2D<float4> InputTex : register( t0 );

sampler BuiltInSampler : register( s0 )
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;    
};

float4 main(PSInputUVOnly Input) : SV_TARGET
{
    float3 Color = InputTex.Sample( BuiltInSampler, Input.UV).rgb;
	return float4(Color, 1.0f);
}