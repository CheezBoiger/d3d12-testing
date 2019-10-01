#pragma once

#include "CommonsD3D11.h"
#include "../Renderer.h"
#include "D3D11Backend.h"

namespace gfx {

class GraphicsCommandListD3D11 : public CommandList
{
public:
  GraphicsCommandListD3D11(D3D11Backend* pBackend)
    : pBackend(pBackend)
    , m_pCmdList(nullptr)
  {
  }

  void init() override {    
    DX11ASSERT(pBackend->getDevice()->CreateDeferredContext(0, &m_ctx));
  }

  void drawInstanced(U32 vertexCountPerInstance, 
                     U32 instanceCount, 
                     U32 startVertexLocation, 
                     U32 startInstanceLocation) override 
  {
    m_ctx->DrawInstanced(vertexCountPerInstance,
                         instanceCount, 
                         startInstanceLocation, 
                         startInstanceLocation);
  }

  void reset() override {
    if (m_pCmdList) {
      m_pCmdList->Release();
    }
  }

  void close() override { 
    m_ctx->FinishCommandList(FALSE, &m_pCmdList); 
  }

  void clearRenderTarget(RenderTargetView* view, 
                         R32* rgba, 
                         U32 numRects, 
                         RECT* rects) override {
    ID3D11RenderTargetView* pView = pBackend->getRenderTargetView(view->getUUID());
    m_ctx->ClearRenderTargetView(pView, rgba);
  }

  ID3D11DeviceContext* m_ctx;
  D3D11Backend* pBackend;
  ID3D11CommandList* m_pCmdList;
};

#if 0
class StaticGraphicsCommandListD3D11 : public GraphicsCommandListD3D11
{
public:
  StaticGraphicsCommandListD3D11(D3D11Backend* pBackend)
    : GraphicsCommandListD3D11(pBackend)
  {
  }

  void setRenderPass(RenderPass* pass) override {
    
  }

  void clearRenderTarget(RenderTargetView* view, 
                         R32* rgba, 
                         U32 numRects, 
                         RECT* rects) override {
    ID3D11RenderTargetView* pView = pBackend->getRenderTargetView(view->getUUID());
    m_ctx->ClearRenderTargetView(pView, rgba);
  }

private:
};
#endif
} // gfx