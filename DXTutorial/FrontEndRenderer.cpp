
#include "FrontEndRenderer.h"
#include "D3D12/D3D12Backend.h"
#include "D3D11/D3D11Backend.h"
#include "GlobalDef.h"

#include <fstream>

namespace jcl {


struct Vertex {
  Vector4 pos;
  Vector4 normal;
  Vector4 tangent;
  Vector4 texCoords;
};


Vertex triangle[3] = {
  { {  1.0f,  0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
  { { -1.0f,  0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
  { {  1.0f,  1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
};


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

  m_pBackend->createBuffer(&pMeshBuffer,
                           gfx::RESOURCE_USAGE_CPU_TO_GPU,
                           gfx::RESOURCE_BIND_CONSTANT_BUFFER,
                           sizeof(PerMeshDescriptor), 
                           0, TEXT("MeshData"));

  m_pBackend->createBuffer(&pOtherMeshBuffer,
                           gfx::RESOURCE_USAGE_CPU_TO_GPU,
                           gfx::RESOURCE_BIND_CONSTANT_BUFFER,
                           sizeof(PerMeshDescriptor), 
                           0, TEXT("OtherMeshData"));

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
  m_pBackend->createRenderTargetView(&m_pAlbedoRenderTargetView,
                                     m_gbuffer.pAlbedoTexture);

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

  m_pBackend->createBuffer(&m_pTriangleVertexBuffer, 
                           gfx::RESOURCE_USAGE_DEFAULT,
                           gfx::RESOURCE_BIND_VERTEX_BUFFER, 
                           sizeof(triangle), 
                           sizeof(Vertex), 
                           TEXT("Triangle"));

  m_pBackend->createVertexBufferView(&m_pTriangleVertexBufferView,
                                     m_pTriangleVertexBuffer,
                                     sizeof(Vertex),
                                     sizeof(triangle));

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
  m_pPreZPass->finalize();

  createGraphicsPipelines();

  {
    gfx::Fence* pFence = nullptr;
    gfx::Resource* pStaging = nullptr;
    gfx::CommandList* pList = nullptr;
    m_pBackend->createBuffer(&pStaging,
                             gfx::RESOURCE_USAGE_CPU_TO_GPU,
                             gfx::RESOURCE_BIND_VERTEX_BUFFER,
                             sizeof(triangle),
                             sizeof(Vertex),
                             TEXT("staging"));

    void* ptr = pStaging->map(0, sizeof(triangle));
    memcpy(ptr, triangle, sizeof(triangle));
    pStaging->unmap(0, sizeof(triangle));

    m_pBackend->createFence(&pFence);
    m_pBackend->createCommandList(&pList);
    pList->init();
    pList->reset();
    pList->copyResource(m_pTriangleVertexBuffer, pStaging);
    pList->close();
    
    m_pBackend->submit(m_pBackend->getSwapchainQueue(), &pList, 1);
    m_pBackend->signalFence(m_pBackend->getSwapchainQueue(), pFence);

    m_pBackend->waitFence(pFence);
    m_pBackend->destroyResource(pStaging);
    m_pBackend->destroyFence(pFence);
    m_pBackend->destroyCommandList(pList);
  }

  m_geometryPass.setGBuffer(&m_gbuffer);
  m_geometryPass.initialize(m_pBackend);
}


void FrontEndRenderer::render()
{
  beginFrame();
  if (m_pList) {
    gfx::Viewport viewport = { };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.w = 1920.0f;
    viewport.h = 1080.0f;
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
    m_pList->clearRenderTarget(m_pAlbedoRenderTargetView, rgba, 1, &rect);
    m_pList->clearDepthStencil(m_pSceneDepthView, 
                               gfx::CLEAR_FLAG_DEPTH,
                               1.0f, 
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
    m_pList->setVertexBuffers(0, &m_pTriangleVertexBufferView, 1);
    m_pList->setIndexBuffer(nullptr);
    for (U32 i = 0; i < 1; ++i) {
      m_pList->setGraphicsRootConstantBufferView(1, pMeshBuffer);
      m_pList->drawInstanced(3, 1, 0, 0);
    }

    m_pList->setGraphicsRootConstantBufferView(1, pOtherMeshBuffer);
    m_pList->drawInstanced(3, 1, 0, 0);

    m_pList->setMarker("GBuffer Pass");

    m_geometryPass.generateCommands(this, m_pList, m_opaqueMeshes.data(), m_opaqueMeshes.size());


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
}


void FrontEndRenderer::cleanUp()
{
  m_pBackend->cleanUp();
}


void FrontEndRenderer::update(R32 dt, Globals& globals)
{
  globals._targetSize[0] = 1920;
  globals._targetSize[1] = 1080;
  globals._targetSize[2] = 0;
  globals._targetSize[3] = 0;
  Matrix44 P = m::Matrix44::perspectiveRH(ToRads(45.0f), 1920.0f / 1080.0f, 0.01f, 1000.0f);
  Matrix44 V = m::Matrix44::lookAtRH(Vector3(3.0f, 0.0f, 3.0f), 
                                     Vector3(0.0f, 0.0f, 0.0f), 
                                     Vector3(0.0f, 1.0f, 0.0f));
  globals._viewToClip = V * P;

  void* pPtr = pGlobalsBuffer->map(0, sizeof(Globals));
  memcpy(pPtr, &globals, sizeof(Globals));
  pGlobalsBuffer->unmap(0, sizeof(Globals));

  mm._worldToViewClip = Matrix44() * V * P;
  mm2._worldToViewClip = Matrix44(
     1.0f, 0.0f, 0.0f,  0.0f,
     0.0f, 1.0f, 0.0f,  0.0f,
     0.0f, 0.0f, 1.0f,  0.0f,
    -1.0f, 0.0f, 0.0f,  1.0f
  ) * V * P;
  

  pPtr = pMeshBuffer->map(0, sizeof(PerMeshDescriptor));
  memcpy(pPtr, &mm, sizeof(PerMeshDescriptor));
  pMeshBuffer->unmap(0, sizeof(PerMeshDescriptor));

  pPtr = pOtherMeshBuffer->map(0, sizeof(PerMeshDescriptor));
  memcpy(pPtr, &mm2, sizeof(PerMeshDescriptor));
  pOtherMeshBuffer->unmap(0, sizeof(PerMeshDescriptor));
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
  info._depthStencilState._depthFunc = gfx::COMPARISON_FUNC_LESS;
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
}


void FrontEndRenderer::retrieveShader(const std::string& filepath,
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
} // jcl