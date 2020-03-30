#pragma once

#include "BackendRenderer.h"
#include "ShadowRenderer.h"

using namespace m;

namespace jcl {
namespace Lights {

struct LightTransform
{
    Matrix44 _viewToClip;
    Matrix44 _clipToView;
};

struct Light
{
    Vector3 _position;
    Vector4 _radiance;
    // Shadow info, otherwise this is left null.
    Shadows::LightShadow* _shadow;
    U32 _transform;
};


struct DirectionLight : public Light 
{
    Vector3 _direction;
};


struct PointLight : public Light 
{
    R32 _radius;
};


struct SpotLight : public Light
{
    Vector3 _direction;
    R32 _inner;
    R32 _outer;
};


class LightSystem
{
public:

    void initialize(gfx::BackendRenderer* pRenderer, 
                    U32 directionLightCount, 
                    U32 pointLightCount, 
                    U32 spotLightCount);

    DirectionLight* getDirectionLight(U32 idx) { return &m_directionLights[idx]; }
    PointLight* getPointLight(U32 idx) { return &m_pointLights[idx]; }
    SpotLight* getSpotLight(U32 idx) { return &m_spotLights[idx]; }
    
    LightTransform* getTransform(U32 idxFromLight) { return &m_lightTransformations[idxFromLight]; }
    void update();

private:
    std::vector<DirectionLight> m_directionLights;
    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
    std::vector<LightTransform> m_lightTransformations;

    gfx::Resource* m_pDirectionLightResource;
    gfx::Resource* m_pPointLightResource;
    gfx::Resource* m_pSpotLightResource;
    gfx::Resource* m_lightTransformsResource;
    gfx::ShaderResourceView* m_pDirectionLightsSRV;
    gfx::ShaderResourceView* m_pSpotLightsSRV;
    gfx::ShaderResourceView* m_pPointLighstSRV;
    gfx::ShaderResourceView* m_pLightTransformSRV;

    void mapLightSystem(void** dirPtr, void** pointPtr, void** spotPtr, void** transformPtr);
    void unmapLightSystem();

    friend gfx::Resource* getLightTransforms(LightSystem*);
    friend gfx::ShaderResourceView* getPointLightsSRV(LightSystem*);
    friend gfx::ShaderResourceView* getSpotLightsSRV(LightSystem*);
    friend gfx::ShaderResourceView* getDirectionLightsSRV(LightSystem*);
};


void initializeLights(gfx::BackendRenderer* pRenderer);

void updateLightRenderer
    (
        // Global constant buffer.
        gfx::Resource* pGlobalConstBuffer,
        // Deferred GBuffer descriptor table.
        GBuffer* gpass,
        // Light system to use.
        LightSystem* pLightSystem
    );

gfx::ShaderResourceView* getLightOutputSRV();

// Render Deferred commands for lights.
void generateDeferredLightsCommands
    (
        // The command list to record.
        gfx::CommandList* pList,
        gfx::Resource* pGlobal
    );

} // Lights
} // jcl