// Shader Code for a pre z pass.
#include "ShaderGlobalDef.h"


struct PSInput 
{
  float4 vPos : SV_POSITION;
  float4 vNormal : NORMAL;
  float4 vTexCoord : TEXCOORD;
};


// PerMesh constant buffer, to be sync'ed with cpu GlobalDef.h struct.
cbuffer PerMesh : register (b0)
{
  float4x4 mWorld;
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