//
#include "FrontEndRenderer.h"
#include "GeometryPass.h"
#include "GraphicsResources.h"

namespace jcl {


void GeometryPass::initialize
    (
        gfx::BackendRenderer* pBackend
    )
{
    pBackend->createRootSignature(&m_pRootSignature);

    gfx::PipelineLayout pLayouts[5] = { };
    pLayouts[0]._numConstantBuffers = 1;
    pLayouts[0]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;

    pLayouts[1]._numConstantBuffers = 1;
    pLayouts[1]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;

    pLayouts[2]._numConstantBuffers = 1;
    pLayouts[2]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;

    pLayouts[3]._numSamplers = 1;
    pLayouts[3]._type = gfx::PIPELINE_LAYOUT_TYPE_SAMPLERS;

    pLayouts[4]._numShaderResourceViews = 4;
    pLayouts[4]._type = gfx::PIPELINE_LAYOUT_TYPE_DESCRIPTOR_TABLE;

    m_pRootSignature->initialize(gfx::SHADER_VISIBILITY_PIXEL | gfx::SHADER_VISIBILITY_VERTEX, pLayouts, 5);

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
    
    pipeInfo._pixelShader._pByteCode = new I8[1024* 1024 * 5];
    pipeInfo._vertexShader._pByteCode = new I8[1024 * 1024 * 5];
    retrieveShader("GeometryTransform.vs.cso", &pipeInfo._vertexShader._pByteCode, pipeInfo._vertexShader._szBytes);
    retrieveShader("GPass.ps.cso", &pipeInfo._pixelShader._pByteCode, pipeInfo._pixelShader._szBytes);

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
    pipeInfo._rasterizationState._cullMode = gfx::CULL_MODE_FRONT;
    pipeInfo._rasterizationState._fillMode = gfx::FILL_MODE_SOLID;
    pipeInfo._rasterizationState._forcedSampleCount = 0;
    pipeInfo._rasterizationState._frontCounterClockwise = true;
    pipeInfo._rasterizationState._depthClipEnable = false;
    pipeInfo._rasterizationState._depthBias = 0.0f;

    pipeInfo._blendState._renderTargets[0]._renderTargetWriteMask = 0xf;
    pipeInfo._blendState._renderTargets[1]._renderTargetWriteMask = 0xf;
    pipeInfo._blendState._renderTargets[2]._renderTargetWriteMask = 0xf;

    // 16bit index buffers passed?
    pipeInfo._ibCutValue = gfx::IB_CUT_VALUE_CUT_0xFFFF;

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
    
    pipeInfo._inputLayout._elementCount = elements.size();
    pipeInfo._inputLayout._pInputElements = elements.data();

    pBackend->createGraphicsPipelineState(&m_pPSO, &pipeInfo);

    delete[] pipeInfo._pixelShader._pByteCode;
    delete[] pipeInfo._vertexShader._pByteCode;

    gfx::SamplerDesc samplerDesc = { };
    samplerDesc._addressU = gfx::SAMPLER_ADDRESS_MODE_CLAMP;
    samplerDesc._addressV = gfx::SAMPLER_ADDRESS_MODE_CLAMP;
    samplerDesc._addressW = samplerDesc._addressV;
    samplerDesc._borderColor[0] = 1.0;
    samplerDesc._borderColor[1] = 1.0f;
    samplerDesc._borderColor[2] = 1.0f;
    samplerDesc._borderColor[3] = 1.0f;
    samplerDesc._comparisonFunc = gfx::COMPARISON_FUNC_ALWAYS;
    samplerDesc._filter = gfx::SAMPLER_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc._maxAnisotropy = 1.0f;
    samplerDesc._mipLodBias = 0.0f;
    samplerDesc._maxLod = 8.0f;
    samplerDesc._minLod = 1.0f;
    pBackend->createSampler(&m_pSampler, &samplerDesc);

    pBackend->createDescriptorTable(&m_pSamplerTable);
    m_pSamplerTable->setSamplers(&m_pSampler, 1);
    m_pSamplerTable->finalize();
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
        GeometryMesh** pMeshes, 
        // Number of meshes in this mesh array.
        U32 meshCount,
        //
        GeometrySubMesh** pSubMeshes,
        //
        U32 submeshCount
    )
{
    if (!_pGBuffer) { 
        DEBUG("No Gbuffer passed.");
        return;
    }
    
    pList->setRenderPass(_pGBuffer->pRenderPass);

    gfx::DescriptorTable* ppTables[] = { pRenderer->getConstBufferDescriptorTable(), m_pSamplerTable };
    // Set the graphics pipeline, assuming we aren't doing any animation skinning, or dynamic mesh rendering,
    // we can just have one pipeline state that is handling static meshes.
    pList->setDescriptorTables(ppTables, 2);
    pList->setGraphicsPipeline(m_pPSO);
    pList->setGraphicsRootSignature(m_pRootSignature);

    U64 submeshIdx = 0;
    for (U32 i = 0; i < meshCount; ++i) {
        RenderUUID meshUUID = pMeshes[i]->_meshDescriptor;
        RenderUUID vertUUID = pMeshes[i]->_vertexBufferView;
        RenderUUID indUUID = pMeshes[i]->_indexBufferView;

        gfx::Resource* pMeshDescriptor = getResource(meshUUID);

        gfx::VertexBufferView* pView = getVertexBufferView(vertUUID);

        pList->setGraphicsRootConstantBufferView(GLOBAL_CONST_SLOT, pRenderer->getGlobalsBuffer());
        pList->setGraphicsRootConstantBufferView(MESH_TRANSFORM_SLOT, pMeshDescriptor);
        pList->setGraphicsRootDescriptorTable(3, m_pSamplerTable);

        pList->setVertexBuffers(0, &pView, 1);

        if (indUUID != 0) 
            pList->setIndexBuffer(getIndexBufferView(indUUID));

        for (U64 j = 0; j < pMeshes[i]->_submeshCount; ++j, ++submeshIdx) {
            RenderUUID matUUID = pSubMeshes[i]->_materialDescriptor;
            gfx::Resource* pMatDescriptor = getResource(matUUID);
            pList->setGraphicsRootConstantBufferView(MATERIAL_DEF_SLOT, pMatDescriptor);
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
}
} // jcl