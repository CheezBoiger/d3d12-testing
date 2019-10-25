//
#include "FrontEndRenderer.h"
#include "GeometryPass.h"

namespace jcl {


void GeometryPass::initialize
    (
        gfx::BackendRenderer* pBackend
    )
{
    pBackend->createRootSignature(&m_pRootSignature);

    gfx::PipelineLayout pLayouts[3] = { };
    pLayouts[0]._numConstantBuffers = 1;
    pLayouts[0]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;

    pLayouts[1]._numConstantBuffers = 1;
    pLayouts[1]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;

    pLayouts[2]._numConstantBuffers = 1;
    pLayouts[2]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;

    m_pRootSignature->initialize(gfx::SHADER_VISIBILITY_PIXEL | gfx::SHADER_VISIBILITY_VERTEX, pLayouts, 3);

    gfx::GraphicsPipelineInfo pipeInfo = { };
    pipeInfo._pRootSignature = m_pRootSignature;
    pipeInfo._dsvFormat = DXGI_FORMAT_D32_FLOAT;
    pipeInfo._sampleMask = 0xffffffff;
    pipeInfo._numRenderTargets = 5;

    // Rendering to the gbuffer, we want to compare geometry to depth, since we are rendering meshes twice, one
    // for the preZ pass, and the second time in the gpass.
    pipeInfo._depthStencilState._stencilEnable = false;
    pipeInfo._depthStencilState._depthEnable = true;
    pipeInfo._depthStencilState._depthFunc = gfx::COMPARISON_FUNC_EQUAL;
    pipeInfo._depthStencilState._depthWriteMask  = gfx::DEPTH_WRITE_MASK_ZERO;

    pipeInfo._pixelShader;
    pipeInfo._vertexShader;

    pipeInfo._topology = gfx::PRIMITIVE_TOPOLOGY_TRIANGLES;
    pipeInfo._rtvFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipeInfo._rtvFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipeInfo._rtvFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipeInfo._rtvFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipeInfo._rtvFormats[4] = DXGI_FORMAT_R16G16_FLOAT;

    pipeInfo._blendState._alphaToCoverageEnable = false;
    pipeInfo._blendState._independentBlendEnable = false;
    pipeInfo._blendState._renderTargets[0]._blendEnable = false;
    pipeInfo._blendState._renderTargets[1]._blendEnable = false;
    pipeInfo._blendState._renderTargets[2]._blendEnable = false;
    pipeInfo._blendState._renderTargets[3]._blendEnable = false;
    pipeInfo._blendState._renderTargets[4]._blendEnable = false;

    pipeInfo._rasterizationState._antialiasedLinesEnable = false;
    pipeInfo._rasterizationState._conservativeRasterizationEnable = false;
    pipeInfo._rasterizationState._cullMode = gfx::CULL_MODE_BACK;
    pipeInfo._rasterizationState._fillMode = gfx::FILL_MODE_SOLID;
    pipeInfo._rasterizationState._forcedSampleCount = 1;
    pipeInfo._rasterizationState._frontCounterClockwise = false;
    pipeInfo._rasterizationState._depthClipEnable = false;
    pipeInfo._rasterizationState._depthBias = 0.0f;

    // 16bit index buffers passed?
    pipeInfo._ibCutValue = gfx::IB_CUT_VALUE_CUT_0xFFFF;

    std::vector<gfx::InputElementInfo> elements(3);
    std::vector<const CHAR*> semantics = { "POSITION", "NORMAL", "TEXCOORD" };
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
    
    pipeInfo._inputLayout._elementCount = elements.size();
    pipeInfo._inputLayout._pInputElements = elements.data();

    pBackend->createGraphicsPipelineState(&m_pPSO, &pipeInfo);
}


void GeometryPass::cleanUp
    (
        gfx::BackendRenderer* pBackend
    )
{

}


void GeometryPass::generateCommands
    ( 
        // Front End renderer.
        FrontEndRenderer* pRenderer,
        // list to record our commands to. 
        gfx::CommandList* pList, 
        // Meshes to render onto this gpass.
        GeometryMesh* pMeshes, 
        // Number of meshes in this mesh array.
        U32 meshCount
    )
{
    if (!_pGBuffer) { 
        DEBUG("No Gbuffer passed.");
        return;
    }
    
    pList->setRenderPass(_pGBuffer->pRenderPass);

    // Set the graphics pipeline, assuming we aren't doing any animation skinning, or dynamic mesh rendering,
    // we can just have one pipeline state that is handling static meshes.
    pList->setGraphicsPipeline(m_pPSO);
    pList->setGraphicsRootSignature(m_pRootSignature);

    for (U32 i = 0; i < meshCount; ++i) {
        RenderUUID meshUUID = pMeshes[i]._meshDescriptor;
        RenderUUID matUUID = pMeshes[i]._materialDescriptor;
        RenderUUID vertUUID = pMeshes[i]._vertexBuffer;
        RenderUUID indUUID = pMeshes[i]._indexBuffer;

        gfx::Resource* pMeshDescriptor = pRenderer->getResource(meshUUID);
        gfx::Resource* pMatDescriptor = pRenderer->getResource(matUUID);

        gfx::VertexBufferView* pView = pRenderer->getVertexBufferView(vertUUID);

        pList->setGraphicsRootConstantBufferView(MESH_TRANSFORM_SLOT, pMeshDescriptor);
        pList->setGraphicsRootConstantBufferView(MATERIAL_DEF_SLOT, pMatDescriptor);

        pList->setVertexBuffers(0, &pView, 1);
        pList->drawInstanced(pMeshes[i]._vertCount, pMeshes[i]._vertInst, 0, 0);
    }
}
} // jcl