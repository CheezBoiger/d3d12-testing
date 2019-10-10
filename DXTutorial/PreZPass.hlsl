
struct PSInput 
{
  float4 vPos : SV_POSITION;
  float4 vNormal : NORMAL;
  float4 vTexCoord : TEXCOORD;
};


float4 main( PSInput input ) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}