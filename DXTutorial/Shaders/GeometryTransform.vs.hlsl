//
#include "ShaderGlobalDef.hlsli"

cbuffer PerMesh : register (b1)
{
    MeshTransforms Mesh;
};


PSInputGeometry main( VSInputGeometry Input )
{
    PSInputGeometry Ps;

    Ps.Position = mul( Mesh.WorldToViewClip, Input.Position );
    Ps.Normal = mul( Mesh.N, Input.Normal ) * 0.5 + 0.5;

    Ps.TexCoords = Input.TexCoord;
    Ps.Tangent = Input.Tangent;

    float4 BiNormal = float4( normalize( cross( Input.Tangent.xyz, Input.Normal.xyz ) ), 0.0 );
    Ps.BiTangent = BiNormal;
    Ps.ScreenPosition = float4( 0.0, 0.0, 0.0, 0.0 );
    Ps.WorldPosition = float4( 0.0, 0.0, 0.0, 0.0 );

	return Ps;
}