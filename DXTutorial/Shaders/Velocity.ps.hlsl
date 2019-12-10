
#include "ShaderGlobalDef.hlsli"

cbuffer Global : register (b0)
{
    GlobalConstants Globals;
};

PSVelocityOutput main(PSInputVelocity Input)
{
    PSVelocityOutput Output;
    float2 a = (Input.ClipPosition.xy / Input.ClipPosition.w) * 0.5 + 0.5;
    float2 b = (Input.PrevClipPosition.xy / Input.PrevClipPosition.w) * 0.5 + 0.5;
    Output.Velocity = float4( a - b , 0.0, 1.0 );
	return Output;
}