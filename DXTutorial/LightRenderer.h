#pragma once

#include "BackendRenderer.h"
#include "FrontEndRenderer.h"
#include "ShadowRenderer.h"

using namespace m;

namespace jcl {
namespace Lights {

struct Light
{
    Vector3 _position;
    Vector4 _radiance;
    // Shadow info, otherwise this is left null.
    Shadows::LightShadow* _shadow;
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

    void initialize(U32 directionLightCount, U32 pointLightCount, U32 spotLightCount);

    DirectionLight* getDirectionLight(U32 idx);
    PointLight* getPointLight(U32 idx);
    SpotLight* getSpotLight(U32 idx);

private:
    std::vector<DirectionLight> m_directionLights;
    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
};


void initializeLights();

// Render Deferred commands for lights.
void generateDeferredLightsCommands
    (
        // The command list to record.
        gfx::CommandList* pList, 
        // Light system to use.
        LightSystem* pLightSystem
    );

} // Lights
} // jcl