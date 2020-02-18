#pragma once


#include "WinConfigs.h"
#include "BackendRenderer.h"
#include "Math/Bounds3D.h"
#include "GraphicsResources.h"
#include "FrontEndRenderer.h"
#include "GlobalDef.h"

namespace jcl {

struct Light;

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
    void update(Light* pLight);

private:
    // Index of the shadow in a given shadow map, depending on if it is within an array.
    U32 m_shadowIdx;
    // Shadow type.
    ShadowType m_type;
    // Shadow resolution.
    ShadowResolution m_shadowResolution;
    // Bounds of the Shadow. To be used for culling.
    Bounds3D m_worldBounds;
};

void initializeShadowRenderer(gfx::BackendRenderer* pRenderer);
void cleanUpShadowRenderer(gfx::BackendRenderer* pRenderer);
// Register the shadow to it's gpu resources.
void registerShadow(LightShadow* shadow);
// Unregister the shadow from the gpu.
void unregisterShadow(LightShadow* shadow);

void generateShadowCommands
    (
        // Front End renderer.
        FrontEndRenderer* pRenderer,
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