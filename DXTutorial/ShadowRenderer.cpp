//
#include "ShadowRenderer.h"
#include "BackendRenderer.h"

namespace jcl {
namespace Shadows {

gfx::Resource* sunlightShadowMapResource;
gfx::Resource* pointLightShadowMapAtlasResource;
gfx::Resource* spotLightShadowMapAtlasResource;

gfx::RenderTargetView* sunlightShadowMapRTV;
gfx::RenderTargetView* pointLightShadowMapAtlasRTV;
gfx::RenderTargetView* spotLightShadowMapAtlasRTV;

gfx::ShaderResourceView* sunlightShadowMapSRV;
gfx::ShaderResourceView* pointLightShadowMapAtlasSRV;
gfx::ShaderResourceView* spotLightShadowMapAtlasSRV;

gfx::GraphicsPipeline* shadowRenderPipeline;

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
    )
{
}


void generateShadowResolveCommand(gfx::CommandList* pList)
{
}


void initializeShadowRenderer(gfx::BackendRenderer* pRenderer)
{
}
} // Shadows
} // jcl