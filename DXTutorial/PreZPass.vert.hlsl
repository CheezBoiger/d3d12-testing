// Shader Code for a pre z pass.
#include "ShaderGlobalDef.h"


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
  float4x4 mInvView;
  float4x4 mProj;
  float4x4 mViewToClip;
  float4x4 mInvProj;
  uint4 uTargetSize;
};

// PerMesh constant buffer, to be sync'ed with cpu GlobalDef.h struct.
cbuffer PerMesh : register (b1)
{
  float4x4 mWorldToViewClip;
  float4x4 mPrevWorldToViewClip;
  float4x4 mN;
  float4 vMaterialFlags;
};


PSInput VSMain( float4 position : POSITION, 
                float4 normal : NORMAL,
                float4 texCoord : TEXCOORD )
{
  PSInput input;
  input.vPos = mul(mWorldToViewClip, position);
  input.vNormal = mul(mN, normal);
  input.vTexCoord = texCoord;

  input.vPos.w = 1.0;
  return input;
}

float4 PSMain( PSInput input ) : SV_TARGET
{

}