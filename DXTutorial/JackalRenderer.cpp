
#include "JackalRenderer.h"
#include "D3D12/D3D12Backend.h"
#include "D3D11/D3D11Backend.h"
#include "GlobalDef.h"

namespace jcl {


void JackalRenderer::init(HWND handle, RendererRHI rhi)
{
  {
    switch (rhi) {
      case RENDERER_RHI_D3D_11:
        m_pBackend = new gfx::D3D11Backend();
        break;
      case RENDERER_RHI_D3D_12:
        m_pBackend = new gfx::D3D12Backend();
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

  pBuffer = nullptr;
  pAlbedo = nullptr;
  m_pBackend->createBuffer(&pBuffer, 
                           gfx::BUFFER_USAGE_CPU_TO_GPU,
                           gfx::BUFFER_BIND_CONSTANT_BUFFER,
                           gfx::BUFFER_DIMENSION_BUFFER,
                           sizeof(Globals),
                           1, 1, 0, DXGI_FORMAT_UNKNOWN, TEXT("Globals"));


  void* ptr = pBuffer->map(0, sizeof(Globals));
  memcpy(ptr, &m_globals, sizeof(Globals));
  pBuffer->unmap(0, sizeof(Globals));

  m_pBackend->createBuffer(&pAlbedo,
                           gfx::BUFFER_USAGE_DEFAULT,
                           gfx::BUFFER_BIND_RENDER_TARGET,
                           gfx::BUFFER_DIMENSION_2D,
                           1920,
                           1080, 1,
                           0, DXGI_FORMAT_R8G8B8A8_UNORM,
                           TEXT("_gbufferAlbedo"));
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

    m_pList->setComputePipeline(nullptr);
    m_pList->dispatch(16, 16, 1);

    m_pList->setRenderPass(nullptr);
    m_pList->setGraphicsPipeline(nullptr);
    m_pList->setVertexBuffers(nullptr, 0);
    m_pList->setIndexBuffer(nullptr);
    m_pList->drawIndexedInstanced(0, 0, 0, 0, 0);

    m_pList->close();
  }
  m_pBackend->submit(m_pBackend->getSwapchainQueue(), &m_pList, 1);
  endFrame();
}


void JackalRenderer::beginFrame()
{
  m_pBackend->waitFence(m_pBackend->getSwapchainFence());
}


void JackalRenderer::endFrame()
{
  m_pBackend->present();
  m_pBackend->signalFence(m_pBackend->getSwapchainQueue(),
                          m_pBackend->getSwapchainFence());
}


void JackalRenderer::cleanUp()
{
  m_pBackend->cleanUp();
}
} // jcl