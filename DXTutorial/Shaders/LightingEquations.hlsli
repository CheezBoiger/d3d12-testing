#ifndef LIGHTING_EQUATIONS_H
#define LIGHTING_EQUATIONS_H

#define JCL_PI 3.14;

struct DirectionLight
{
    float4 WorldPos;
    float4 Dir;
    float4 Color;
};


struct PointLight
{
    float4 WorldPos;
    float3 Color;
    float Radius;
};


struct SpotLight
{
    float4 WorldPos;
    float4 Color;
    float3 Dir;
    float Length;
};


struct LightBuffer
{
    DirectionLight DirectionLights[1];
    PointLight PointLights[16];
    SpotLight SpotLights[16];
};


float GGX(float NoH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = ( NoH * NoH ) * ( alpha - 1.0 ) + 1.0;
    return alpha2 / ( 3.14 * ( denom * denom ) );
}


float GGXSchlickApprox(float NoV, float roughness)
{
    float remap = roughness + 1.0;
    float k = ( remap * remap ) / 8.0;
    float num = NoV;
    float denom = ( NoV * ( 1.0 - k ) + k );
    return num / denom;
}


float SchlickSmithGGX(float NoL, float NoV, float roughness)
{
    float ggx1 = GGXSchlickApprox( NoL, roughness );
    float ggx2 = GGXSchlickApprox( NoV, roughness );
    return ggx1 * ggx2;
}


float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + ( 1.0 - F0 ) * pow( 1.0 - cosTheta, 5.0 );
}


float3 LambertDiffuse(float kD, float3 Albedo)
{
    return kD * Albedo / JCL_PI;
}


float3 BRDF(float D, float3 F, float G, float NoL, float NoV)
{
    float3 brdf = D * F * G / ( 4 * NoL * NoV );
    return brdf;
}


// Calculating the Direct Illumination of a directional light source.
float3 directionLightRadiance(float3 V, float3 Albedo, float3 N, float roughness, float metallic, float3 F0, DirectionLight Light)
{
    float3 Color = float3( 0, 0, 0 );

    // Reverse the light direction to get from fragment depth to light source vector.
    float3 L = normalize( -Light.Dir.xyz );
    float3 Radiance = Light.Color.xyz;

    // Half vector.
    float3 H = normalize( L + V );
    
    float NoL = clamp( dot(N, L), 0.001, 1.0 );
    float NoV = clamp( abs( dot( N, V ) ), 0.001, 1.0 );
    float NoH = clamp( dot( N, H ), 0.001, 1.0 );
    float VoH = clamp( dot( V, H ), 0.001, 1.0 );

    if (NoL > 0) {
        
    }
    return Color;
}
#endif