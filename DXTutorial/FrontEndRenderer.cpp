
#include "FrontEndRenderer.h"
#include "D3D12/D3D12Backend.h"
#include "D3D11/D3D11Backend.h"
#include "GlobalDef.h"
#include "VelocityRenderer.h"
#include "GraphicsResources.h"
#include "DebugGUI.h"

#include <fstream>

namespace jcl {


void FrontEndRenderer::init(HWND handle, RendererRHI rhi)
{
  {
    switch (rhi) {
      case RENDERER_RHI_D3D_11:
        m_pBackend = gfx::getBackendD3D11();
        break;
      case RENDERER_RHI_D3D_12:
        m_pBackend = gfx::getBackendD3D12();
        break;
      case RENDERER_RHI_NULL:
      default:
        m_pBackend = new gfx::BackendRenderer();
    }
  }

    m_pGlobals = nullptr;

  gfx::GpuConfiguration config = { };
  config._desiredBuffers = 2;
  config._enableVSync = true;
  config._renderHeight = 1080;
  config._renderWidth = 1920;
  config._windowed = true;
  m_pBackend->initialize(handle, false, config);

    if (m_pBackend->isHardwareRaytracingCompatible()) {
    
    }

  if (m_pBackend->isHardwareMachineLearningCompatible()) {
    
  }

  m_pBackend->createCommandList(&m_pList);

  if (m_pList)
    m_pList->init();

  pGlobalsBuffer = nullptr;
  m_gbuffer.pAlbedoTexture = nullptr;
  m_pBackend->createBuffer(&pGlobalsBuffer, 
                           gfx::RESOURCE_USAGE_CPU_TO_GPU,
                           gfx::RESOURCE_BIND_CONSTANT_BUFFER,
                           sizeof(Globals),
                           0, TEXT("Globals"));

  m_pBackend->createTexture(&m_gbuffer.pAlbedoTexture,
                           gfx::RESOURCE_DIMENSION_2D,
                           gfx::RESOURCE_USAGE_DEFAULT,
                           gfx::RESOURCE_BIND_RENDER_TARGET,
                           DXGI_FORMAT_R8G8B8A8_UNORM,
                           1920,
                           1080, 
                           1,
                           0,
                           TEXT("GBufferAlbedo"));
    m_pBackend->createTexture(&m_gbuffer.pNormalTexture,
                                gfx::RESOURCE_DIMENSION_2D,
                                gfx::RESOURCE_USAGE_DEFAULT,
                                gfx::RESOURCE_BIND_RENDER_TARGET | gfx::RESOURCE_BIND_SHADER_RESOURCE,
                                DXGI_FORMAT_R8G8B8A8_UNORM,
                                1920,
                                1080,
                                1, 0, TEXT("GBufferNormal"));
    m_pBackend->createShaderResourceView(&m_gbuffer.pNormalSRV,
                                            m_gbuffer.pNormalTexture,
                                            0, 1);
  m_pBackend->createRenderTargetView(&m_gbuffer.pAlbedoRTV,
                                     m_gbuffer.pAlbedoTexture);
    m_pBackend->createRenderTargetView(&m_gbuffer.pNormalRTV,
                                        m_gbuffer.pNormalTexture);

  m_pBackend->createDescriptorTable(&m_pConstBufferTable);

  m_pConstBufferTable->setConstantBuffers(&pGlobalsBuffer, 1);
  m_pConstBufferTable->finalize();

  m_pRootSignature = nullptr;
  m_pBackend->createRootSignature(&m_pRootSignature);
  std::vector<gfx::PipelineLayout> layouts(2);
  layouts[0]._numConstantBuffers = 1;
  layouts[0]._numSamplers = 0;
  layouts[0]._numShaderResourceViews = 0;
  layouts[0]._numUnorderedAcessViews = 0;

  layouts[1]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;
  layouts[1]._numConstantBuffers = 1;
  layouts[1]._numSamplers = 0;
  layouts[1]._numShaderResourceViews = 0;
  layouts[1]._numUnorderedAcessViews = 0;

  m_pRootSignature->initialize(gfx::SHADER_VISIBILITY_PIXEL | gfx::SHADER_VISIBILITY_VERTEX, 
                               layouts.data(), 
                               2);

  m_pBackend->createTexture(&m_pSceneDepth,
                            gfx::RESOURCE_DIMENSION_2D,
                            gfx::RESOURCE_USAGE_DEFAULT,
                            gfx::RESOURCE_BIND_DEPTH_STENCIL,
                            DXGI_FORMAT_D32_FLOAT,
                            1920,
                            1080, 1, 0, TEXT("SceneDepth"));
  m_pBackend->createDepthStencilView(&m_pSceneDepthView, 
                                     m_pSceneDepth);

  m_pBackend->createRenderPass(&m_pPreZPass, 0, true);
  m_pPreZPass->setDepthStencil(m_pSceneDepthView);

    createFinalRootSignature();
    createGraphicsPipelines();
    createComputePipelines();
    // Set the scene depth view to be used as read only in gbuffer pass.
    m_pBackend->createRenderPass(&m_gbuffer.pRenderPass, 4, true);
    gfx::RenderTargetView* rtvs[] = { m_gbuffer.pAlbedoRTV, m_gbuffer.pNormalRTV };
    m_gbuffer.pRenderPass->setRenderTargets(rtvs, 2);
    m_gbuffer.pRenderPass->setDepthStencil(m_pSceneDepthView);
    m_geometryPass.setGBuffer(&m_gbuffer);
    m_geometryPass.initialize(m_pBackend);

    initializeVelocityRenderer(m_pBackend, m_pSceneDepthView);
}


void FrontEndRenderer::render()
{
  beginFrame();
  if (m_pList) {
    gfx::Viewport viewport = { };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.w = (R32)m_pGlobals->_targetSize[0];
    viewport.h = (R32)m_pGlobals->_targetSize[1];
    viewport.mind = 0.0f;
    viewport.maxd = 1.0f;
  
    gfx::Scissor scissor = { };
    scissor.top = 0;
    scissor.right = 1920;
    scissor.bottom = 1080;
    scissor.left = 0;

    static R32 t = 0.0f;
    R32 v = sinf(t * 0.05f);
    R32 s = -sinf(t * 0.05f);
    ++t;
    R32 rgba[] = {s, v, 0.f, 0.f};
    RECT rect = {};
    rect.bottom = 1080;
    rect.left = 0;
    rect.right = 1920;
    rect.top = 0;

    m_pList->reset("Cool Comamand List");
    m_pList->clearRenderTarget(m_pBackend->getSwapchainRenderTargetView(), rgba,
                               1, &rect);
    m_pList->clearRenderTarget(m_gbuffer.pAlbedoRTV, rgba, 1, &rect);
    m_pList->clearDepthStencil(m_pSceneDepthView, 
                               gfx::CLEAR_FLAG_DEPTH,
                               0.0f, 
                               0, 1, &rect);

    //m_pList->setComputePipeline(nullptr);
    //m_pList->dispatch(16, 16, 1);

    m_pList->setMarker("PreZPass");
    m_pList->setViewports(&viewport, 1);
    m_pList->setScissors(&scissor, 1);
    m_pList->setDescriptorTables(&m_pConstBufferTable, 1);
    m_pList->setGraphicsRootSignature(m_pRootSignature);
    m_pList->setGraphicsRootDescriptorTable(GLOBAL_CONST_SLOT, m_pConstBufferTable);

    m_pList->setRenderPass(m_pPreZPass);
    m_pList->setGraphicsPipeline(m_pPreZPipeline);

    for (U32 i = 0; i < m_opaqueMeshes.size(); ++i) {
        RenderUUID meshId = m_opaqueMeshes[i]->_meshDescriptor;
        RenderUUID vertId = m_opaqueMeshes[i]->_vertexBufferView;
        RenderUUID indId = m_opaqueMeshes[i]->_indexBufferView;
        gfx::Resource* pMeshDescriptor = getResource(meshId);
        gfx::VertexBufferView* view = getVertexBufferView(vertId);

        m_pList->setVertexBuffers(0, &view, 1);
        m_pList->setGraphicsRootConstantBufferView(1, pMeshDescriptor);

        if (indId != 0) {
            m_pList->setIndexBuffer(getIndexBufferView(indId));
            m_pList->drawIndexedInstanced(  m_opaqueMeshes[i]->_indCount, 
                                            m_opaqueMeshes[i]->_vertInst, 
                                            m_opaqueMeshes[i]->_indOffset, 
                                            m_opaqueMeshes[i]->_startVert, 0);
        } else {
            m_pList->drawInstanced(m_opaqueMeshes[i]->_vertCount, m_opaqueMeshes[i]->_vertInst, m_opaqueMeshes[i]->_startVert, 0);
        }
    }

    //m_pList->setGraphicsRootConstantBufferView(1, pOtherMeshBuffer);
    //m_pList->drawInstanced(3, 1, 0, 0);

    m_pList->setMarker("GBuffer Pass");

    m_geometryPass.generateCommands(this, m_pList, m_opaqueMeshes.data(), m_opaqueMeshes.size());
    submitVelocityCommands(m_pBackend, pGlobalsBuffer, m_pList, m_opaqueMeshes.data(), m_opaqueMeshes.size());

    m_pList->setMarker("Debug GUI");
    populateCommandListGUI(m_pBackend, m_pList);

    m_pList->setMarker("Final Backbuffer Pass");
    m_pList->setRenderPass(m_pBackend->getBackbufferRenderPass());
    m_pList->setDescriptorTables(&m_pFinalDescriptorTable, 1);
    m_pList->setGraphicsRootSignature(m_pFinalRootSig);
    m_pList->setGraphicsRootDescriptorTable(0, m_pFinalDescriptorTable);
    m_pList->setGraphicsPipeline(m_pFinalBackBufferPipeline);
    m_pList->drawInstanced(3, 1, 0, 0);

    m_pList->close();
  }
  m_pBackend->submit(m_pBackend->getSwapchainQueue(), &m_pList, 1);
  endFrame();
}


void FrontEndRenderer::beginFrame()
{
}


void FrontEndRenderer::endFrame()
{
    m_pBackend->present();
    m_opaqueMeshes.clear();
    m_transparentMeshes.clear();
}


void FrontEndRenderer::cleanUp()
{
  m_pBackend->cleanUp();
}


void FrontEndRenderer::update(R32 dt, Globals& globals)
{
    void* pMatPtr = nullptr;
    void* pPtr = pGlobalsBuffer->map(0, sizeof(Globals));
    memcpy(pPtr, m_pGlobals, sizeof(Globals));
    pGlobalsBuffer->unmap(0, sizeof(Globals));

    for (U64 i = 0; i < m_opaqueMeshes.size(); ++i) {
        gfx::Resource* pDescriptor = getResource(m_opaqueMeshes[i]->_meshDescriptor);
        gfx::Resource* pMatDescriptor = getResource(m_opaqueMeshes[i]->_materialDescriptor);
        pPtr = pDescriptor->map(0, sizeof(PerMeshDescriptor));
        pMatPtr = pMatDescriptor->map(0, sizeof(PerMaterialDescriptor));
        memcpy(pPtr, m_opaqueMeshes[i]->_meshTransform, sizeof(PerMeshDescriptor));
        memcpy(pMatPtr, m_opaqueMeshes[i]->_matData, sizeof(PerMaterialDescriptor));
        pDescriptor->unmap(0, sizeof(PerMeshDescriptor));
        pMatDescriptor->unmap(0, sizeof(PerMaterialDescriptor));
        
    }
}


void FrontEndRenderer::createGraphicsPipelines()
{
  m_pPreZPipeline = nullptr;
  gfx::GraphicsPipelineInfo info = { };
  gfx::ShaderByteCode vertBytecode;
  gfx::ShaderByteCode pixBytecode;
  vertBytecode._pByteCode = new I8[1024 * 1024 * 5];
  pixBytecode._pByteCode = new I8[1024 * 1024 * 5];
  retrieveShader("PreZPass.vs.cso",
                 &vertBytecode._pByteCode,
                 vertBytecode._szBytes);
  retrieveShader("PreZPass.ps.cso",
                 &pixBytecode._pByteCode,
                 pixBytecode._szBytes);

  info._vertexShader = vertBytecode;
  //info._pixelShader = pixBytecode;  
  info._numRenderTargets = 0;
  info._sampleMask = 0xffffffff;
  info._depthStencilState._backFace._stencilDepthFailOp = gfx::STENCIL_OP_ZERO;
  info._depthStencilState._backFace._stencilFailOp = gfx::STENCIL_OP_ZERO;
  info._depthStencilState._backFace._stencilFunc = gfx::COMPARISON_FUNC_NEVER;
  info._depthStencilState._backFace._stencilPassOp = gfx::STENCIL_OP_KEEP;
  info._depthStencilState._stencilEnable = false;
  info._depthStencilState._frontFace = info._depthStencilState._backFace;

  info._depthStencilState._depthEnable = true;
  info._depthStencilState._depthFunc = gfx::COMPARISON_FUNC_GREATER;
  info._depthStencilState._depthWriteMask = gfx::DEPTH_WRITE_MASK_ALL;

  info._depthStencilState._stencilReadMask = 0x1;
  info._depthStencilState._stencilWriteMask = 0xff;

  info._rasterizationState._antialiasedLinesEnable = false;
  info._rasterizationState._conservativeRasterizationEnable = false;
  info._rasterizationState._cullMode = gfx::CULL_MODE_BACK;
  info._rasterizationState._depthBias = 0.0f;
  info._rasterizationState._depthBiasClamp = 0.0f;
  info._rasterizationState._depthClipEnable = true;
  info._rasterizationState._fillMode = gfx::FILL_MODE_SOLID;
  info._rasterizationState._forcedSampleCount = 0;
  info._rasterizationState._frontCounterClockwise = false;
  info._rasterizationState._slopedScaledDepthBias = 0.0f;
 
  info._topology = gfx::PRIMITIVE_TOPOLOGY_TRIANGLES;
  info._dsvFormat = DXGI_FORMAT_D32_FLOAT;
  info._pRootSignature = m_pRootSignature;

  info._blendState._renderTargets[0]._blendEnable = false;
  info._blendState._renderTargets[0]._logicOpEnable = false;
  info._blendState._renderTargets[0]._blendOp = gfx::BLEND_OP_ADD;
  info._blendState._renderTargets[0]._blendOpAlpha = gfx::BLEND_OP_ADD;
  info._blendState._renderTargets[0]._logicOp = gfx::LOGIC_OP_NOOP;
  info._blendState._renderTargets[0]._renderTargetWriteMask = gfx::COLOR_WRITE_ENABLE_ALL;
  

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

  info._inputLayout._elementCount = elements.size();
  info._inputLayout._pInputElements = elements.data();

  m_pBackend->createGraphicsPipelineState(&m_pPreZPipeline, &info);

  delete[] vertBytecode._pByteCode;
  delete[] pixBytecode._pByteCode;

    // Final pipeline for quad rendering.
    createFinalPipeline();
}


void retrieveShader(const std::string& filepath,
                                    void** bytecode,
                                    size_t& length)
{
  std::ifstream fileinput(filepath, std::ifstream::ate | std::ifstream::binary);
  if (!fileinput.is_open()) {
    DEBUG("Failed to load shader!");
    length = 0;
    return;
  } 

  length = size_t(fileinput.tellg());
  fileinput.seekg(0);
  
  fileinput.read((I8*)*bytecode, length);
  fileinput.close();
}


RenderUUID FrontEndRenderer::createTexture(gfx::ResourceDimension dimension, gfx::ResourceUsage usage, gfx::ResourceBindFlags binds, DXGI_FORMAT format, U64 width, U64 height, U64 depth, U64 strideBytes, const TCHAR* debugName)
{
    gfx::Resource* pResource = nullptr;
    m_pBackend->createTexture(  &pResource, 
                                dimension, 
                                usage, 
                                binds, 
                                format, 
                                width, 
                                height, 
                                depth, 
                                strideBytes, 
                                debugName); 
    return cacheResource(pResource);
}


RenderUUID FrontEndRenderer::createBuffer(gfx::ResourceUsage usage, gfx::ResourceBindFlags flags, U64 sz, U64 strideBytes, const TCHAR* debug)
{
    gfx::Resource* pResource = nullptr;
    m_pBackend->createBuffer(&pResource, usage, flags, sz, strideBytes, debug);
    return cacheResource(pResource);
}


VertexBuffer FrontEndRenderer::createVertexBuffer(void* meshRaw, U64 vertexSzBytes, U64 meshSzBytes)
{
    VertexBuffer vertexBuffer = { 0, 0 };
    gfx::Resource* vertexMesh = nullptr;
    gfx::VertexBufferView* vertexBufferView = nullptr;

    m_pBackend->createBuffer(&vertexMesh, 
                            gfx::RESOURCE_USAGE_DEFAULT, 
                            gfx::RESOURCE_BIND_VERTEX_BUFFER, 
                            meshSzBytes, 
                            vertexSzBytes,
                            TEXT("VertBuffer"));
    m_pBackend->createVertexBufferView( &vertexBufferView,
                                         vertexMesh,
                                         vertexSzBytes,
                                         meshSzBytes);
  {
    gfx::Fence* pFence = nullptr;
    gfx::Resource* pStaging = nullptr;
    gfx::CommandList* pList = nullptr;
    m_pBackend->createBuffer(&pStaging,
                             gfx::RESOURCE_USAGE_CPU_TO_GPU,
                             gfx::RESOURCE_BIND_VERTEX_BUFFER,
                             meshSzBytes,
                             vertexSzBytes,
                             TEXT("staging"));

    void* ptr = pStaging->map(0, meshSzBytes);
    memcpy(ptr, meshRaw, meshSzBytes);
    pStaging->unmap(0, meshSzBytes);

    m_pBackend->createFence(&pFence);
    m_pBackend->createCommandList(&pList);
    pList->init();
    pList->reset();
    pList->copyResource(vertexMesh, pStaging);
    pList->close();
    
    m_pBackend->submit(m_pBackend->getSwapchainQueue(), &pList, 1);
    m_pBackend->signalFence(m_pBackend->getSwapchainQueue(), pFence);

    m_pBackend->waitFence(pFence);
    m_pBackend->destroyResource(pStaging);
    m_pBackend->destroyFence(pFence);
    m_pBackend->destroyCommandList(pList);
  }


    RenderUUID vertId = cacheResource(vertexMesh);
    RenderUUID viewId = cacheVertexBufferView(vertexBufferView);

    vertexBuffer.resource = vertId;
    vertexBuffer.vertexBufferView = viewId;

    return vertexBuffer;
}


RenderUUID FrontEndRenderer::createTransformBuffer()
{
    gfx::Resource* pResource = nullptr;
    m_pBackend->createBuffer(&pResource,
                               gfx::RESOURCE_USAGE_CPU_TO_GPU,
                               gfx::RESOURCE_BIND_CONSTANT_BUFFER,
                               sizeof(PerMeshDescriptor), 
                               0, TEXT("MeshTranform"));
    return cacheResource(pResource);
}


RenderUUID FrontEndRenderer::createMaterialBuffer()
{
    gfx::Resource* pResource = nullptr;
    m_pBackend->createBuffer(&pResource,
                               gfx::RESOURCE_USAGE_CPU_TO_GPU,
                               gfx::RESOURCE_BIND_CONSTANT_BUFFER,
                               sizeof(PerMaterialDescriptor), 
                               0, TEXT("MaterialDescription"));
    return cacheResource(pResource);
 }


void FrontEndRenderer::createComputePipelines()
{
    m_pBackend->createRootSignature(&m_pBitonicSortSig);
    gfx::PipelineLayout pLayouts[2] = { };
    pLayouts[0]._type = gfx::PIPELINE_LAYOUT_TYPE_UAV;
    pLayouts[0]._numUnorderedAcessViews = 1;

    pLayouts[1]._type = gfx::PIPELINE_LAYOUT_TYPE_CBV;
    pLayouts[1]._numConstantBuffers = 1;

    m_pBitonicSortSig->initialize(gfx::SHADER_VISIBILITY_ALL, pLayouts, 2);

    gfx::ShaderByteCode bytecode = { };
    bytecode._pByteCode = new U8[1024 * 1024 * 2];
    retrieveShader("BitonicSort.cs.cso", &bytecode._pByteCode, bytecode._szBytes);
    gfx::ComputePipelineInfo info = { };
    info._pRootSignature = m_pBitonicSortSig;
    info._computeShader = bytecode;
    m_pBackend->createComputePipelineState(&m_bitonicSort, &info);
    
    //retrieveShader("Reflection.cs.cso", &bytecode._pByteCode, bytecode._szBytes);
    //m_pBackend->createComputePipelineState(&m_pReflectionPipeline, &info);
    
    //retrieveShader("ScreenSpaceReflections.cs.cso", &bytecode._pByteCode, bytecode._szBytes);
    //m_pBackend->createComputePipelineState(&m_pSSRPipeline, &info);

    delete[] bytecode._pByteCode;
}


IndexBuffer FrontEndRenderer::createIndexBufferView(void* raw, U64 szBytes)
{
    IndexBuffer b = { };
    RenderUUID res = createBuffer(  gfx::RESOURCE_USAGE_DEFAULT,
                    gfx::RESOURCE_BIND_INDEX_BUFFER,
                    szBytes,
                    0,
                    TEXT("SceneIndexBuffer"));

    {
        gfx::Fence* pFence = nullptr;
        gfx::Resource* pStaging = nullptr;
        gfx::CommandList* pList = nullptr;
        m_pBackend->createBuffer(&pStaging,
                                    gfx::RESOURCE_USAGE_CPU_TO_GPU,
                                    gfx::RESOURCE_BIND_INDEX_BUFFER,
                                    szBytes,
                                    sizeof(U32),
                                    TEXT("staging"));

        void* ptr = pStaging->map(0, szBytes);
        memcpy(ptr, raw, szBytes);
        pStaging->unmap(0, szBytes);

        m_pBackend->createFence(&pFence);
        m_pBackend->createCommandList(&pList);
        pList->init();
        pList->reset();
        pList->copyResource(getResource(res), pStaging);
        pList->close();
    
        m_pBackend->submit(m_pBackend->getSwapchainQueue(), &pList, 1);
        m_pBackend->signalFence(m_pBackend->getSwapchainQueue(), pFence);

        m_pBackend->waitFence(pFence);
        m_pBackend->destroyResource(pStaging);
        m_pBackend->destroyFence(pFence);
        m_pBackend->destroyCommandList(pList);
    }

    gfx::IndexBufferView* view = nullptr;
    m_pBackend->createIndexBufferView(&view, getResource(res), DXGI_FORMAT_R32_UINT, szBytes);
    RenderUUID viewId = cacheIndexBufferView(view);

    b.indexBufferView = viewId;
    b.resource = res;
    return b;
}


RenderUUID FrontEndRenderer::createTexture2D(U64 width, U64 height, void* pData, DXGI_FORMAT format)
{
    gfx::Resource* pResource = nullptr;
    m_pBackend->createTexture(&pResource,
                                gfx::RESOURCE_DIMENSION_2D,
                                gfx::RESOURCE_USAGE_DEFAULT,
                                gfx::RESOURCE_BIND_SHADER_RESOURCE,
                                format,
                                width, height);
    {
        gfx::Fence* pFence = nullptr;
        gfx::Resource* pStaging = nullptr;
        gfx::CommandList* pList = nullptr;
        U64 szBytes = width * height * 4;
        m_pBackend->createBuffer(&pStaging,
            gfx::RESOURCE_USAGE_CPU_TO_GPU,
            gfx::RESOURCE_BIND_SHADER_RESOURCE,
            szBytes,
            0,
            TEXT("staging"));

        void* ptr = pStaging->map(0, szBytes);
        memcpy(ptr, pData, szBytes);
        pStaging->unmap(0, szBytes);

        m_pBackend->createFence(&pFence);
        m_pBackend->createCommandList(&pList);
        pList->init();
        pList->reset();
        pList->copyResource(pResource, pStaging);
        pList->close();

        m_pBackend->submit(m_pBackend->getSwapchainQueue(), &pList, 1);
        m_pBackend->signalFence(m_pBackend->getSwapchainQueue(), pFence);

        m_pBackend->waitFence(pFence);
        m_pBackend->destroyResource(pStaging);
        m_pBackend->destroyFence(pFence);
        m_pBackend->destroyCommandList(pList);
    }

    gfx::ShaderResourceView* pView = nullptr;
    m_pBackend->createShaderResourceView(&pView, pResource, 0, width * height);
    RenderUUID id = cacheResource(pResource);
    return id;
}


void FrontEndRenderer::createFinalRootSignature()
{
    gfx::PipelineLayout layouts[1] = { };
    layouts[0]._type = gfx::PIPELINE_LAYOUT_TYPE_DESCRIPTOR_TABLE;
    layouts[0]._numShaderResourceViews = 1;
    
    gfx::StaticSamplerDesc staticSampler = { };
    staticSampler._addressU = gfx::SAMPLER_ADDRESS_MODE_WRAP;
    staticSampler._addressV = staticSampler._addressU;
    staticSampler._addressW = staticSampler._addressV;
    staticSampler._comparisonFunc = gfx::COMPARISON_FUNC_ALWAYS;
    staticSampler._filter = gfx::SAMPLER_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    staticSampler._registerSpace = 0;
    staticSampler._shaderRegister = 0;
    staticSampler._minLod = 1.0f;
    staticSampler._maxLod = 8.0f;
    staticSampler._maxAnisotropy = 1.0f;

    m_pBackend->createRootSignature(&m_pFinalRootSig);
    m_pFinalRootSig->initialize(gfx::SHADER_VISIBILITY_VERTEX | gfx::SHADER_VISIBILITY_PIXEL,
                                layouts, 1, &staticSampler, 1);

    m_pBackend->createDescriptorTable(&m_pFinalDescriptorTable);
    m_pFinalDescriptorTable->setShaderResourceViews(&m_gbuffer.pNormalSRV, 1);
    m_pFinalDescriptorTable->finalize();
}


void FrontEndRenderer::createFinalPipeline()
{
    gfx::GraphicsPipelineInfo gInfo = { };
    gInfo._blendState._renderTargets[0]._blendEnable = false;
    gInfo._blendState._renderTargets[0]._renderTargetWriteMask = 0xf;
    
    gInfo._depthStencilState._depthEnable = false;
    gInfo._depthStencilState._stencilEnable = false;

    gInfo._numRenderTargets = 1;
    gInfo._rtvFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    
    gInfo._inputLayout._elementCount = 0;
    gInfo._inputLayout._pInputElements = nullptr;
    
    gInfo._pRootSignature = m_pFinalRootSig;
    gInfo._sampleMask = 0xffffffff;
    gInfo._dsvFormat = DXGI_FORMAT_D32_FLOAT;
    
    gInfo._topology = gfx::PRIMITIVE_TOPOLOGY_TRIANGLES;
    gInfo._rasterizationState._fillMode = gfx::FILL_MODE_SOLID;
    gInfo._rasterizationState._forcedSampleCount = 0;
    gInfo._rasterizationState._frontCounterClockwise = false;
    gInfo._rasterizationState._cullMode = gfx::CULL_MODE_BACK;

    gfx::ShaderByteCode vB = { };
    gfx::ShaderByteCode pB = { };

    vB._pByteCode = new U8[KB_1 * 64];
    pB._pByteCode = new U8[KB_1 * 64];

    retrieveShader("Composite.ps.cso", &pB._pByteCode, pB._szBytes);
    retrieveShader("Quad.vs.cso", &vB._pByteCode, vB._szBytes);
    
    gInfo._vertexShader = vB;
    gInfo._pixelShader = pB;
    
    m_pBackend->createGraphicsPipelineState(&m_pFinalBackBufferPipeline, &gInfo);

    delete[] vB._pByteCode;
    delete[] pB._pByteCode;
    
}
} // jcl