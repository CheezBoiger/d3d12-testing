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

gfx::RootSignature* shadowRootSignature;

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

void createShadowRootSignature(gfx::BackendRenderer* pRenderer)
{
    pRenderer->createRootSignature(&shadowRootSignature);
    gfx::PipelineLayout layouts[4];
    layouts[0] = { };
    layouts[0]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;
    layouts[0]._numConstantBuffers = 1;

    layouts[1] = { };
    layouts[1]._numConstantBuffers = 1;
    layouts[1]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;

    layouts[2] = { };
    layouts[2]._type = gfx::PIPELINE_LAYOUT_TYPE_DESCRIPTOR_TABLE;
    layouts[2]._numShaderResourceViews = 1;

    layouts[3] = { };
    layouts[3]._type = gfx::PIPELINE_LAYOUT_TYPE_SAMPLERS;
    layouts[3]._numSamplers = 1;
    
    shadowRootSignature->initialize(gfx::SHADER_VISIBILITY_VERTEX | gfx::SHADER_VISIBILITY_PIXEL,
                                    layouts, 4);
}

void createShadowMapPipeline(gfx::BackendRenderer* pRenderer)
{
    std::vector<gfx::InputElementInfo> elements(4);
    std::vector<const CHAR*> semantics = { "POSITION", "NORMAL", "TANGENT", "TEXCOORD" };
    U32 offset = 0;
    for (size_t i = 0; i < elements.size(); ++i) {
        elements[i]._alignedByteOffset = offset;
        elements[i]._classification = gfx::INPUT_CLASSIFICATION_PER_VERTEX;
        elements[i]._format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        elements[i]._instanceDataStepRate = 0;
        elements[i]._semanticIndex = 0;
        elements[i]._semanticName = semantics[i];
        offset += sizeof(Vector4);
    }

    gfx::GraphicsPipelineInfo info = { };
    info._blendState._alphaToCoverageEnable = false;
    info._blendState._independentBlendEnable = false;
    info._blendState._renderTargets[0] = { };
    info._blendState._renderTargets[0]._renderTargetWriteMask = gfx::COLOR_WRITE_ENABLE_ALL;
    info._blendState._renderTargets[0]._blendOp = gfx::BLEND_OP_ADD;
    info._blendState._renderTargets[0]._blendOpAlpha = gfx::BLEND_OP_ADD;
    info._blendState._renderTargets[0]._logicOp = gfx::LOGIC_OP_NOOP;
    info._blendState._renderTargets[0]._blendEnable = false;
    info._blendState._renderTargets[0]._logicOpEnable = false;
    info._numRenderTargets = 0;
    info._depthStencilState._backFace._stencilDepthFailOp = gfx::STENCIL_OP_ZERO;
    info._depthStencilState._depthFunc = gfx::COMPARISON_FUNC_LESS;
    info._depthStencilState._depthWriteMask = gfx::DEPTH_WRITE_MASK_ALL;
    info._depthStencilState._depthEnable = true;
    info._depthStencilState._frontFace = { };
    info._depthStencilState._stencilEnable = false;
    info._depthStencilState._stencilReadMask = 0x1;
    info._depthStencilState._stencilWriteMask = 0xff;
    // 16 bit unorm for shadow maps, as they don't need to be insanely precise,
    // But applications will want better precision depending on the situation.
    info._dsvFormat = DXGI_FORMAT_D32_FLOAT;
    info._ibCutValue = gfx::IB_CUT_VALUE_DISABLED;
    info._inputLayout._elementCount = elements.size();
    info._inputLayout._pInputElements = elements.data();
    info._sampleMask = 0xffffffff;
    info._topology = gfx::PRIMITIVE_TOPOLOGY_TRIANGLES;
    info._rasterizationState._antialiasedLinesEnable = false;
    info._rasterizationState._conservativeRasterizationEnable = false;
    info._rasterizationState._cullMode = gfx::CULL_MODE_BACK;
    info._rasterizationState._fillMode = gfx::FILL_MODE_SOLID;
    info._rasterizationState._frontCounterClockwise = true;
    info._rasterizationState._depthBiasClamp = 0.f;
    info._rasterizationState._depthBias = 0;
    info._rasterizationState._depthClipEnable = true;
    info._rasterizationState._slopedScaledDepthBias = 0.f;
    info._rasterizationState._forcedSampleCount = 0;
    info._pRootSignature = shadowRootSignature;

    //info._pixelShader._pByteCode = new U8[1024 * 1024 * 5];
    //info._pixelShader._szBytes = 0;
    info._vertexShader._pByteCode = new U8[1024 * 1024 * 5];
    //retrieveShader("Depth.ps.cso", &info._pixelShader._pByteCode, info._pixelShader._szBytes);
    retrieveShader("Depth.vs.cso", &info._vertexShader._pByteCode, info._vertexShader._szBytes);

    pRenderer->createGraphicsPipelineState(&shadowRenderPipeline, &info);

    delete[] info._pixelShader._pByteCode;
    delete[] info._vertexShader._pByteCode;
}

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
        U32 submeshCount,
        //
        gfx::Resource* pGlobal,
        Lights::LightSystem* pLightSystem
    )
{
    gfx::Resource* pTransforms = getLightTransforms(pLightSystem);
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
        gfx::Viewport viewport = { };
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.w = 512.f;
        viewport.h = 512.f;
        viewport.mind = 1.f;
        viewport.maxd = 0.f;
        gfx::Scissor scissor = { };
        scissor.bottom = 512.f;
        scissor.right = 512.0f;
        scissor.left = 0.f;
        scissor.top = 0.f;
        pList->setViewports(&viewport, 1);
        pList->setScissors(&scissor, 1);
        pList->setRenderPass(directionLightRenderPass);
        pList->clearDepthStencil(directionLightDSV, gfx::CLEAR_FLAG_DEPTH, 1.0f, 0, 1, &rect);
        pList->setGraphicsRootSignature(shadowRootSignature);
        pList->setGraphicsPipeline(shadowRenderPipeline);
        pList->setGraphicsRootConstantBufferView(1, pTransforms, 256 * i);

        // Set up resources for this shadow.
        U64 submeshIdx = 0;
        for (U32 i = 0; i < meshCount; ++i) {
            RenderUUID meshUUID = pMeshes[i]->_meshTransform;
            RenderUUID vertUUID = pMeshes[i]->_vertexBufferView;
            RenderUUID indUUID = pMeshes[i]->_indexBufferView;

            gfx::Resource* pMeshDescriptor = getResource(meshUUID);
            pList->setGraphicsRootConstantBufferView(0, pMeshDescriptor);
            gfx::VertexBufferView* pView = getVertexBufferView(vertUUID);

            pList->setVertexBuffers(0, &pView, 1);

            if (indUUID != 0) 
                pList->setIndexBuffer(getIndexBufferView(indUUID));

            for (U64 j = 0; j < pMeshes[i]->_submeshCount; ++j, ++submeshIdx) {
                RenderUUID matUUID = pSubMeshes[submeshIdx]->_materialDescriptor;
                gfx::Resource* pMatDescriptor = getResource(matUUID);
                if (pSubMeshes[submeshIdx]->_matData->_matrialFlags & MATERIAL_USE_ALBEDO_MAP) { }
                if (indUUID != 0) {
                    pList->drawIndexedInstanced(pSubMeshes[submeshIdx]->_indCount, 
                                                pSubMeshes[submeshIdx]->_vertInst, 
                                                pSubMeshes[submeshIdx]->_indOffset, 
                                                pSubMeshes[submeshIdx]->_startVert, 0);
                } else {
                    pList->drawInstanced(pSubMeshes[submeshIdx]->_vertCount, 
                                            pSubMeshes[submeshIdx]->_vertInst, 
                                            pSubMeshes[submeshIdx]->_startVert, 0);
                }
            }
        }
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
                             gfx::RESOURCE_BIND_DEPTH_STENCIL | gfx::RESOURCE_BIND_SHADER_RESOURCE,
                             DXGI_FORMAT_R32_TYPELESS,
                             512u, 512u, 8u, 0, TEXT("DirectionLightShadowAtlas"));
    // 16MB
    pRenderer->createTexture(&pointLightShadowMapAtlasResource,
                             gfx::RESOURCE_DIMENSION_2D,
                             gfx::RESOURCE_USAGE_DEFAULT,
                             gfx::RESOURCE_BIND_DEPTH_STENCIL | gfx::RESOURCE_BIND_SHADER_RESOURCE,
                             DXGI_FORMAT_R16_TYPELESS,
                             512u, 512u, 32u, 0u, TEXT("PointLightShadowAtlas"));
    // 16MB
    pRenderer->createTexture(&spotLightShadowMapAtlasResource,
                             gfx::RESOURCE_DIMENSION_2D,
                             gfx::RESOURCE_USAGE_DEFAULT,
                             gfx::RESOURCE_BIND_DEPTH_STENCIL | gfx::RESOURCE_BIND_SHADER_RESOURCE,
                             DXGI_FORMAT_R16_TYPELESS,
                             512u, 512u, 32u, 0u, TEXT("SpotLightShadowAtlas"));
    createShadowRootSignature(pRenderer);
    createShadowMapPipeline(pRenderer);
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
                desc._format = DXGI_FORMAT_D32_FLOAT;
                desc._dimension = gfx::RESOURCE_DIMENSION_2D;
                desc._texture2D._mipSlice = index; // MipSlice + ( ArraySlice * MipLevels )
                //desc._flags = gfx::DEPTH_STENCIL_FLAG_ONLY_DEPTH;
                pRenderer->createDepthStencilView(&dsv, directionLightShadowMapAtlasResource, desc);
                directionLightDSVs.push_back(dsv);
                gfx::RenderPass* rp = nullptr;
                pRenderer->createRenderPass(&rp, 0, true);
                rp->setDepthStencil(dsv);
                directionLightRenderPasses.push_back(rp);
            } break;
        case LightShadow::SHADOW_TYPE_OMNIDIRECTIONAL:
            {
                pointLightShadows.push_back(shadow);
                index = static_cast<U32>(pointLightShadows.size() - 1ull);
                desc._texture2D._mipSlice = index;
                //desc._flags = gfx::DEPTH_STENCIL_FLAG_ONLY_DEPTH;
                desc._dimension = gfx::RESOURCE_DIMENSION_2D;
                pRenderer->createDepthStencilView(&dsv, pointLightShadowMapAtlasResource, desc);
                pointLightDSVs.push_back(dsv);
                gfx::RenderPass* rp = nullptr;
                pRenderer->createRenderPass(&rp, 0, true);
                rp->setDepthStencil(dsv);
            } break;
        case LightShadow::SHADOW_TYPE_SPOT:
            {
                spotLightShadows.push_back(shadow);
                index = static_cast<U32>(spotLightShadows.size() - 1ull);
                desc._texture2D._mipSlice = index;
                //desc._flags = gfx::DEPTH_STENCIL_FLAG_ONLY_DEPTH;
                desc._dimension = gfx::RESOURCE_DIMENSION_2D;
                pRenderer->createDepthStencilView(&dsv, spotLightShadowMapAtlasResource, desc);
                spotLightDSVs.push_back(dsv);
                gfx::RenderPass* rp = nullptr;
                pRenderer->createRenderPass(&rp, 0, true);
                rp->setDepthStencil(dsv);
            } break;
        default: break;
    }
    
    setLightShadowIndex(shadow, index);
}


void LightShadow::update(Lights::Light* light, Lights::LightTransform* pTransform)
{       
    if (!light || !pTransform) return;
    Matrix44 mProjection;
    Matrix44 mView;
    switch (m_type) {
        case SHADOW_TYPE_DIRECTIONAL:
            {
                Lights::DirectionLight* dirLight = static_cast<Lights::DirectionLight*>(light);
                
                mProjection = Matrix44::orthographicRH(40.f, 40.f, 0.1, 100.0f);
                mView = Matrix44::lookAtRH(dirLight->_position + Vector3(10.f, -10.f, 10.f), 
                                           dirLight->_direction, 
                                           Vector3(0.0f, 1.f, 0.0f));
            } break;
    }

    m_viewToClip = mView * mProjection; 
    
    pTransform->_viewToClip = m_viewToClip;
    pTransform->_clipToView = m_viewToClip.inverse();
    m_lightTransformIdx = light->_transform;
    m_dirty = true;
}
} // Shadows
} // jcl