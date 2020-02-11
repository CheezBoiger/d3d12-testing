//
#include "VelocityRenderer.h"
#include "BackendRenderer.h"
#include "GraphicsResources.h"

#include <array>

namespace jcl {


gfx::GraphicsPipeline* pPipelineVelocity = nullptr;
gfx::GraphicsPipeline* pPipelineVelocityResolve = nullptr;

gfx::RootSignature* pVelocityRootSig = nullptr;
gfx::RootSignature* pVelocityResolveRootSig = nullptr;

gfx::Resource* pVelocityTexture = nullptr;
gfx::RenderTargetView* pVelocityRenderTargetView = nullptr;

gfx::RenderPass* pVelocityRenderPass = nullptr;


void initializePipeline(gfx::BackendRenderer* pRenderer)
{
    gfx::GraphicsPipelineInfo velocityPipelineInfo = { };
    velocityPipelineInfo._pRootSignature = pVelocityRootSig;
    velocityPipelineInfo._blendState._renderTargets[0]._blendEnable = false;
    velocityPipelineInfo._blendState._renderTargets[0]._renderTargetWriteMask = 0xf;

    velocityPipelineInfo._depthStencilState._depthEnable = true;
    velocityPipelineInfo._depthStencilState._depthWriteMask = gfx::DEPTH_WRITE_MASK_ZERO;
    velocityPipelineInfo._depthStencilState._stencilEnable = false;
    velocityPipelineInfo._depthStencilState._depthFunc = gfx::COMPARISON_FUNC_EQUAL;

    velocityPipelineInfo._topology = gfx::PRIMITIVE_TOPOLOGY_TRIANGLES;
    velocityPipelineInfo._dsvFormat = DXGI_FORMAT_D32_FLOAT;
    velocityPipelineInfo._numRenderTargets = 1;
    velocityPipelineInfo._rtvFormats[0] = DXGI_FORMAT_R32G32_FLOAT;

    velocityPipelineInfo._sampleMask = 0xffffffff;
    
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

    velocityPipelineInfo._inputLayout._elementCount = elements.size();
    velocityPipelineInfo._inputLayout._pInputElements = elements.data();

    velocityPipelineInfo._rasterizationState._antialiasedLinesEnable = false;
    velocityPipelineInfo._rasterizationState._conservativeRasterizationEnable = false;
    velocityPipelineInfo._rasterizationState._cullMode = gfx::CULL_MODE_BACK;
    velocityPipelineInfo._rasterizationState._fillMode = gfx::FILL_MODE_SOLID;
    velocityPipelineInfo._rasterizationState._forcedSampleCount = 0;
    velocityPipelineInfo._rasterizationState._frontCounterClockwise = true;
    velocityPipelineInfo._rasterizationState._depthClipEnable = false;
    velocityPipelineInfo._rasterizationState._depthBias = 0.0f;

    velocityPipelineInfo._ibCutValue = gfx::IB_CUT_VALUE_CUT_0xFFFF;

    gfx::ShaderByteCode vertexShader = { };
    gfx::ShaderByteCode pixelShader = { };

    vertexShader._pByteCode = new U8[1024 * 64];
    pixelShader._pByteCode = new U8[1024 * 64];

    retrieveShader("Velocity.vs.cso", &vertexShader._pByteCode, vertexShader._szBytes);
    retrieveShader("Velocity.ps.cso", &pixelShader._pByteCode, pixelShader._szBytes);

    velocityPipelineInfo._pixelShader = pixelShader;
    velocityPipelineInfo._vertexShader = vertexShader;

    pRenderer->createGraphicsPipelineState(&pPipelineVelocity, &velocityPipelineInfo);

    delete[] vertexShader._pByteCode;
    delete[] pixelShader._pByteCode;
}


void initializeRootSignature(gfx::BackendRenderer* pRenderer)
{
    pRenderer->createRootSignature(&pVelocityRootSig);
    pRenderer->createRootSignature(&pVelocityResolveRootSig);
    std::array<gfx::PipelineLayout, 2> layout;

    layout[0] = { };
    layout[0]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;
    layout[0]._numConstantBuffers = 1;

    layout[1] = { };
    layout[1]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;
    layout[1]._numConstantBuffers = 1;


    pVelocityRootSig->initialize(gfx::SHADER_VISIBILITY_PIXEL | gfx::SHADER_VISIBILITY_VERTEX, layout.data(), layout.size());
    
    std::array<gfx::PipelineLayout, 3> resolveLayout;
    //resolveLayout
    //pVelocityResolveRootSig->initialize(gfx::SHADER_VISIBILITY_PIXEL, resolveLayout.data(), resolveLayout.size());
}


void initializeRenderPass(gfx::BackendRenderer* pRenderer, gfx::DepthStencilView* pDepth)
{
    pRenderer->createRenderPass(&pVelocityRenderPass, 1, true);

    pVelocityRenderPass->setDepthStencil(pDepth);
    pVelocityRenderPass->setRenderTargets(&pVelocityRenderTargetView, 1);

}


void initializeRenderTarget(gfx::BackendRenderer* pRenderer)
{
    pRenderer->createTexture(&pVelocityTexture, 
                            gfx::RESOURCE_DIMENSION_2D, 
                            gfx::RESOURCE_USAGE_DEFAULT, 
                            gfx::RESOURCE_BIND_RENDER_TARGET, 
                            DXGI_FORMAT_R32G32_FLOAT, 
                            1920, 1080, 1,
                            0, TEXT("VelocityTexture"));
    pRenderer->createRenderTargetView(&pVelocityRenderTargetView, pVelocityTexture);
}


void initializeVelocityRenderer(gfx::BackendRenderer* pRenderer, gfx::DepthStencilView* pDepth)
{
    initializeRootSignature(pRenderer);
    initializePipeline(pRenderer);
    initializeRenderTarget(pRenderer);
    initializeRenderPass(pRenderer, pDepth);
}


void cleanUpVelocityRenderer(gfx::BackendRenderer* pRenderer)
{

}


void submitVelocityCommands(gfx::BackendRenderer* pRenderer, 
                            gfx::Resource* pGlobal, 
                            gfx::CommandList* pList, 
                            GeometryMesh** pMeshes, 
                            U32 meshCount,
                            GeometrySubMesh** pSubMeshes,
                            U32 submeshCount)
{
    pList->setMarker("Velocity");

    pList->setRenderPass(pVelocityRenderPass);
    R32 rgba[] = { 0, 0, 0, 0 };
    RECT rect = { 0, 0, 1920, 1080 };
    gfx::Viewport viewport = { };
    viewport.h = 1080.0f;
    viewport.w = 1920.0f;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.mind = 0.0f;
    viewport.maxd = 1.0f;
    pList->clearRenderTarget(pVelocityRenderTargetView, rgba, 1, &rect);
    pList->setViewports(&viewport, 1);

    pList->setGraphicsRootSignature(pVelocityRootSig);
    pList->setGraphicsRootConstantBufferView(GLOBAL_CONST_SLOT, pGlobal);
    pList->setGraphicsPipeline(pPipelineVelocity);

    U32 submeshIdx = 0;
    for (U32 i = 0; i < meshCount; ++i) {
        GeometryMesh* pMesh = pMeshes[i];
        RenderUUID meshId = pMesh->_meshDescriptor;
        RenderUUID vertId = pMesh->_vertexBufferView;
        RenderUUID indId = pMesh->_indexBufferView;
        gfx::Resource* pMeshDescriptor = getResource(meshId);
        gfx::VertexBufferView* vb = getVertexBufferView(vertId);
        
        pList->setVertexBuffers(0, &vb, 1);
        pList->setGraphicsRootConstantBufferView(MESH_TRANSFORM_SLOT, pMeshDescriptor);

        if (indId != 0)
            pList->setIndexBuffer(getIndexBufferView(indId));

        for (U32 j = 0; j < pMesh->_submeshCount; ++j, ++submeshIdx) {
                if (indId != 0) {
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

    pList->setMarker(nullptr);
}

 } // jcl