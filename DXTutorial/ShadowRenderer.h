#pragma once


#include "WinConfigs.h"
#include "BackendRenderer.h"
#include "Math/Bounds3D.h"
#include "GraphicsResources.h"
#include "FrontEndRenderer.h"
#include "GlobalDef.h"

namespace jcl {
namespace Shadows {


// Light Shadow is the object that holds the info to the shadow atlas, along with other 
// info regarding the shadow to be drawn.
struct LightShadow
{
    // Bounds of the Shadow. To be used for culling.
    Bounds3D _worldBounds;

    struct {
        // The perspective of the light.
        Matrix44 _viewToClipSpace;
        // The id of the shadow in the atlas.
        U32 id;
    } _toGpu;
};

void initializeShadowRenderer(gfx::BackendRenderer* pRenderer);
void cleanUpShadowRenderer(gfx::BackendRenderer* pRenderer);

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
// Generate the shadow resolve. This is to be used for sunlight only!
// 
void generateShadowResolveCommand(gfx::CommandList* pList);

// Retrieve the shadow atlas used by the lighting stage.
gfx::Resource* getShadowAtlas();
gfx::RenderTargetView* getShadowResolve();
} // Shadows
} // jcl