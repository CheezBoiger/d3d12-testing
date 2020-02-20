//
#include "ShadowRenderer.h"
#include "BackendRenderer.h"
#include "LightRenderer.h"

namespace jcl {
namespace Shadows {

gfx::Resource* sunlightShadowMapCascadeResource;
gfx::Resource* directionLightShadowMapAtlasResource;
gfx::Resource* pointLightShadowMapAtlasResource;
gfx::Resource* spotLightShadowMapAtlasResource;

gfx::RenderTargetView* sunlightShadowMapRTV;
gfx::RenderTargetView* pointLightShadowMapAtlasRTV;
gfx::RenderTargetView* spotLightShadowMapAtlasRTV;

gfx::ShaderResourceView* sunlightShadowMapSRV;
gfx::ShaderResourceView* pointLightShadowMapAtlasSRV;
gfx::ShaderResourceView* spotLightShadowMapAtlasSRV;

gfx::GraphicsPipeline* shadowRenderPipeline;

std::vector<LightShadow*> pointLightShadows;
std::vector<LightShadow*> directionLightShadows;
std::vector<LightShadow*> spotLightShadows;

std::vector<gfx::DepthStencilView*> pointLightDSVs;
std::vector<gfx::DepthStencilView*> directionLightDSVs;
std::vector<gfx::DepthStencilView*> spotLightDSVs;

std::vector<gfx::RenderPass*> pointLightRenderPasses;
std::vector<gfx::RenderPass*> directionLightRenderPasses;
std::vector<gfx::RenderPass*> spotLightRenderPasses;

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
    // Direction light shadow map check and render.
    for (U32 i = 0; i < directionLightShadows.size(); ++i) {
        LightShadow* shadowInfo = directionLightShadows[i];
        
        if (!shadowInfo->needsUpdate()) 
            continue;
        
        U32 shadowIdx = shadowInfo->getShadowIndex();
        gfx::DepthStencilView* directionLightDSV = directionLightDSVs[shadowIdx];
        gfx::RenderPass* directionLightRenderPass = directionLightRenderPasses[shadowIdx];
        RECT rect = { };
        rect.bottom = 512;
        rect.right = 512;
        rect.left = rect.top = 0; 

        pList->setRenderPass(directionLightRenderPass);
        pList->clearDepthStencil(directionLightDSV, gfx::CLEAR_FLAG_DEPTH, 0.0f, 0, 1, &rect);
        pList->setGraphicsPipeline(shadowRenderPipeline);

        // Set up resources for this shadow.
        
        // Signal the shadowmap is no longer in need of rerendering.
        signalClean(shadowInfo);
    }
}


void signalClean(LightShadow* lightShadow)
{
    if (!lightShadow) return;
    lightShadow->m_dirty = false;
}


void setLightShadowIndex(LightShadow* lightShadow, U32 idx)
{
    if (!lightShadow) return;
    lightShadow->m_shadowIdx = idx;
}


void generateShadowResolveCommand(gfx::CommandList* pList)
{
}


void initializeShadowRenderer(gfx::BackendRenderer* pRenderer)
{
    // 4MB
    pRenderer->createTexture(&directionLightShadowMapAtlasResource, 
                             gfx::RESOURCE_DIMENSION_2D,
                             gfx::RESOURCE_USAGE_DEFAULT,
                             gfx::RESOURCE_BIND_RENDER_TARGET | gfx::RESOURCE_BIND_SHADER_RESOURCE,
                             DXGI_FORMAT_R16_TYPELESS,
                             512u, 512u, 8u, 0, TEXT("DirectionLightShadowAtlas"));
    // 16MB
    pRenderer->createTexture(&pointLightShadowMapAtlasResource,
                             gfx::RESOURCE_DIMENSION_2D,
                             gfx::RESOURCE_USAGE_DEFAULT,
                             gfx::RESOURCE_BIND_RENDER_TARGET | gfx::RESOURCE_BIND_SHADER_RESOURCE,
                             DXGI_FORMAT_R16_TYPELESS,
                             512u, 512u, 32u, 0u, TEXT("PointLightShadowAtlas"));
    // 16MB
    pRenderer->createTexture(&spotLightShadowMapAtlasResource,
                             gfx::RESOURCE_DIMENSION_2D,
                             gfx::RESOURCE_USAGE_DEFAULT,
                             gfx::RESOURCE_BIND_RENDER_TARGET | gfx::RESOURCE_BIND_SHADER_RESOURCE,
                             DXGI_FORMAT_R16_TYPELESS,
                             512u, 512u, 32u, 0u, TEXT("SpotLightShadowAtlas"));
}


void registerShadow(gfx::BackendRenderer* pRenderer, LightShadow* shadow)
{
    gfx::DepthStencilViewDesc desc;
    gfx::DepthStencilView* dsv = nullptr;
    desc._texture2D._mipSlice = 0;
    U32 index = 0;

    switch (shadow->getShadowType()) {
        case LightShadow::SHADOW_TYPE_DIRECTIONAL:
            {
                directionLightShadows.push_back(shadow);
                index = static_cast<U32>(directionLightShadows.size() - 1ull);
                desc._format = DXGI_FORMAT_D16_UNORM;
                desc._dimension = gfx::RESOURCE_DIMENSION_2D;
                desc._texture2D._mipSlice = index; // MipSlice + ( ArraySlice * MipLevels )
                desc._flags = gfx::DEPTH_STENCIL_FLAG_ONLY_DEPTH;
                pRenderer->createDepthStencilView(&dsv, directionLightShadowMapAtlasResource, desc);
                directionLightDSVs.push_back(dsv);
            } break;
        case LightShadow::SHADOW_TYPE_OMNIDIRECTIONAL:
            {
                pointLightShadows.push_back(shadow);
                index = static_cast<U32>(pointLightShadows.size() - 1ull);
                desc._texture2D._mipSlice = index;
                desc._flags = gfx::DEPTH_STENCIL_FLAG_ONLY_DEPTH;
                desc._dimension = gfx::RESOURCE_DIMENSION_2D;
                pRenderer->createDepthStencilView(&dsv, pointLightShadowMapAtlasResource, desc);
                pointLightDSVs.push_back(dsv);
            } break;
        case LightShadow::SHADOW_TYPE_SPOT:
            {
                spotLightShadows.push_back(shadow);
                index = static_cast<U32>(spotLightShadows.size() - 1ull);
                desc._texture2D._mipSlice = index;
                desc._flags = gfx::DEPTH_STENCIL_FLAG_ONLY_DEPTH;
                desc._dimension = gfx::RESOURCE_DIMENSION_2D;
                pRenderer->createDepthStencilView(&dsv, spotLightShadowMapAtlasResource, desc);
                spotLightDSVs.push_back(dsv);
            } break;
        default: break;
    }
    
    setLightShadowIndex(shadow, index);
}


void LightShadow::update(Lights::Light* light)
{       
    if (!light) return;
    Matrix44 mProjection;
    Matrix44 mView;
    switch (m_type) {
        case SHADOW_TYPE_DIRECTIONAL:
            {
                Lights::DirectionLight* dirLight = static_cast<Lights::DirectionLight*>(light);
                
                mProjection = Matrix44::orthographicRH(512.f, 512.f, 0.1, 100.0f);
                
                mView = Matrix44::lookAtRH(dirLight->_position, 
                                           dirLight->_direction + dirLight->_position, 
                                           Vector3(0.0f, 1.f, 0.0f));
            } break;
    }

    m_viewToClip = mView * mProjection;   
}
} // Shadows
} // jcl