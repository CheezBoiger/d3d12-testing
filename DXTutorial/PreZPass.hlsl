
struct PSInput 
{
  float4 vPos : SV_POSITION;
  float4 vNormal : NORMAL;
  float4 vTexCoord : TEXCOORD;
};


// Global Info, to be sync'ed with cpu GlobalDef.h struct
cbuffer Global : register (b0)
{
    float4 vCameraPos;
    float4x4 mViewToWorld;
    float4x4 mWorldToView;
    float4x4 mProj;
    float4x4 mViewToClip;
    float4x4 mClipToView;
    uint4 uTargetSize;
};


float4 main( PSInput input ) : SV_TARGET
{
	return vCameraPos.xyzw;
}