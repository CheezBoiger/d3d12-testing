
#include "JackalRenderer.h"
#include "D3D12/D3D12Backend.h"
#include "D3D11/D3D11Backend.h"

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
  m_pBackend->createCommandList(&m_pList, gfx::COMMAND_LIST_RECORD_USAGE_SIMULTANEOUS);


  m_pList->init();
}


void JackalRenderer::render()
{
  beginFrame();
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
  m_pList->setRenderPass(m_pBackend->getBackbufferRenderPass());
  m_pList->clearRenderTarget(m_pBackend->getSwapchainRenderTargetVew(), rgba, 1,
                             &rect);
  m_pList->close();


  m_pBackend->submit(m_pBackend->getSwapchainQueue(), &m_pList, 1);
  endFrame();
}


void JackalRenderer::beginFrame()
{
}


void JackalRenderer::endFrame()
{
  m_pBackend->present();
  m_pBackend->signalFence(m_pBackend->getSwapchainQueue(),
                          m_pBackend->getSwapchainFence());
  m_pBackend->waitFence(m_pBackend->getSwapchainFence());
}


void JackalRenderer::cleanUp()
{
  m_pBackend->cleanUp();
}
} // jcl