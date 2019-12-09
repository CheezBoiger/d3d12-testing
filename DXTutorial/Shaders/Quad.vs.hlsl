#include "ShaderGlobalDef.hlsli"

PSInputUVOnly main( uint vertID : SV_VERTEXID )
{
    PSInputUVOnly Output;  
    Output.UV = float2((vertID << 1) & 2, vertID & 2);
    Output.Pos = float4(Output.UV * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
	return Output;
}