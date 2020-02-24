#pragma once


#include "WinConfigs.h"
#include "BackendRenderer.h"
#include "Math/Bounds3D.h"
#include "GraphicsResources.h"
#include "GlobalDef.h"
#include "Math/Plane.h"

namespace jcl {

namespace Lights {
struct Light;
}

namespace Shadows {

// Shadow Resolution directs the shadow to be rendered on the defined shadow map.
// The Shadow Renderer takes the responsibility of optimizing the shadow depending on the 
// resolution and type.
enum ShadowResolution
{
    SHADOW_RESOLUTION_512_512,
    SHADOW_RESOLUTION_1024_1024,
    SHADOW_RESOLUTION_2048_2048,
    SHADOW_RESOLUTION_4096_4096,
};


// Light Shadow is the object that holds the info to the shadow atlas, along with other 
// info regarding the shadow to be drawn.
class LightShadow
{
public:
    enum ShadowType
    {
        SHADOW_TYPE_DIRECTIONAL,
        SHADOW_TYPE_OMNIDIRECTIONAL,
        SHADOW_TYPE_SPOT
    };

    void initialize(ShadowType type, ShadowResolution resolution);
    // Update with the given light info.
    void update(Lights::Light* pLight);

    ShadowType getShadowType() const { return m_type; }
    // 6 Planes corresponding to each side of the view frustum.
    const Plane* getViewFrustumPlanes() const { return m_planes; }
    U32 getShadowIndex() const { return m_shadowIdx; }
    Matrix44 getViewToClip() const { return m_viewToClip; }

    B32 needsUpdate() const { return m_dirty; }

private:
    // Index of the shadow in a given shadow map, depending on if it is within an array.
    U32 m_shadowIdx;
    // Shadow type.
    ShadowType m_type;
    // Shadow resolution.
    ShadowResolution m_shadowResolution;
    // View Frustum Planes.
    Plane m_planes[6];
    // View to Clip matrix.
    Matrix44 m_viewToClip;

    // Dirty flag.
    B32 m_dirty;

    friend void setLightShadowIndex(LightShadow* lightShadow, U32 idx);
    friend void signalClean(LightShadow* lightShadow);
};

void initializeShadowRenderer(gfx::BackendRenderer* pRenderer);
void cleanUpShadowRenderer(gfx::BackendRenderer* pRenderer);
// Register the shadow to it's gpu resources.
void registerShadow(gfx::BackendRenderer* pRenderer, LightShadow* shadow);
// Unregister the shadow from the gpu.
void unregisterShadow(LightShadow* shadow);

void generateShadowCommands
    (
        // list to record our commands to. 
        gfx::CommandList* pList,
        // Meshes to render onto this gpass.
        GeometryMesh** pMeshes,
        // Number of meshes in this mesh array.
        U32 meshCount,
        //
        GeometrySubMesh** pSubMeshes,
        //
        U32 submeshCount
    );
// Generate the shadow resolve. This is to be used for one direction light only!
// 
void generateShadowResolveCommand(gfx::CommandList* pList);

// Retrieve the shadow atlas used by the lighting stage.
gfx::Resource* getShadowAtlas();
gfx::RenderTargetView* getShadowResolve();
} // Shadows
} // jcl