//
#include "ShaderGlobalDef.hlsli"

cbuffer PerMesh : register (b1)
{
    MeshTransforms Mesh;
};


PSInputGeometry main( VSInputGeometry Input )
{
    PSInputGeometry Ps;

    Ps.Position = mul( Input.Position, Mesh.WorldToViewClip );
    Ps.Normal = mul( Input.Normal, Mesh.N );

    Ps.TexCoords = Input.TexCoord;
    Ps.Tangent = Input.Tangent;

    float4 BiNormal = float4( normalize( cross( Input.Tangent.xyz, Input.Normal.xyz ) ), 0.0 );
    Ps.BiTangent = BiNormal;
    Ps.ScreenPosition = float4( 0.0, 0.0, 0.0, 0.0 );
    Ps.WorldPosition = float4( 0.0, 0.0, 0.0, 0.0 );

	return Ps;
}