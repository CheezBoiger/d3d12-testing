
#include "ShaderGlobalDef.hlsli"

// Global Info, to be sync'ed with cpu GlobalDef.h struct
cbuffer Global : register (b0)
{
    GlobalConstants GlobalData;
};


float4 main ( PSInputBasic input ) : SV_TARGET
{
	return GlobalData.vCameraPos.xyzw;
}