
#include "JackalRenderer.h"
#include "D3D12/D3D12Backend.h"
#include "D3D11/D3D11Backend.h"
#include "GlobalDef.h"

namespace jcl {


struct Vertex {
  Vector4 pos;
  Vector4 texCoords;
  Vector4 color;
};


Vertex triangle[3] = {
  { { 1.0f,  0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
  { { 0.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
  { { 0.5f,  0.5f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
};


void JackalRenderer::init(HWND handle, RendererRHI rhi)
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
  m_pBackend->createRenderTargetView(&m_pAlbedoRenderTargetView,
                                     m_gbuffer.pAlbedoTexture);

  m_pBackend->createDescriptorTable(&m_pConstBufferTable);

  m_pConstBufferTable->setConstantBuffers(&pGlobalsBuffer, 1);
  m_pConstBufferTable->finalize();

  void* ptr = pGlobalsBuffer->map(0, sizeof(Globals));
  memcpy(ptr, &m_globals, sizeof(Globals));
  pGlobalsBuffer->unmap(0, sizeof(Globals));

  m_pRootSignature = nullptr;
  m_pBackend->createRootSignature(&m_pRootSignature);
  gfx::PipelineLayout layout = { };
  layout.numConstantBuffers = 1;
  layout.numSamplers = 0;
  layout.numShaderResourceViews = 0;
  layout.numUnorderedAcessViews = 0;
  m_pRootSignature->initialize(gfx::SHADER_VISIBILITY_PIXEL | gfx::SHADER_VISIBILITY_VERTEX, 
                               &layout, 
                               1);

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
                            DXGI_FORMAT_D24_UNORM_S8_UINT,
                            1920,
                            1080, 1, 0, TEXT("SceneDepth"));
  m_pBackend->createDepthStencilView(&m_pSceneDepthResourceView, 
                                     m_pSceneDepth);
  
#if 1
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
#endif
}


void JackalRenderer::render()
{
  beginFrame();
  if (m_pList) {
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

    m_pList->reset();
    m_pList->clearRenderTarget(m_pBackend->getSwapchainRenderTargetView(), rgba,
                               1, &rect);
    m_pList->clearRenderTarget(m_pAlbedoRenderTargetView, rgba, 1, &rect);
    m_pList->clearDepthStencil(m_pSceneDepthResourceView, 
                               gfx::CLEAR_FLAG_DEPTH | gfx::CLEAR_FLAG_STENCIL,
                               0.0f, 
                               0, 1, &rect);

    m_pList->setComputePipeline(nullptr);
    m_pList->dispatch(16, 16, 1);

    m_pList->setDescriptorTables(&m_pConstBufferTable, 1);
    m_pList->setGraphicsRootSignature(m_pRootSignature);
    m_pList->setRenderPass(nullptr);
    m_pList->setGraphicsPipeline(nullptr);
    m_pList->setVertexBuffers(0, &m_pTriangleVertexBufferView, 1);
    m_pList->setIndexBuffer(nullptr);
    m_pList->drawIndexedInstanced(0, 0, 0, 0, 0);

    m_pList->close();
  }
  m_pBackend->submit(m_pBackend->getSwapchainQueue(), &m_pList, 1);
  endFrame();
}


void JackalRenderer::beginFrame()
{
}


void JackalRenderer::endFrame()
{
  m_pBackend->present();
}


void JackalRenderer::cleanUp()
{
  m_pBackend->cleanUp();
}
} // jcl