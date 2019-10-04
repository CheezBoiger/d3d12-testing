#pragma once

#include "CommonsD3D11.h"
#include "../Renderer.h"
#include "D3D11Backend.h"
#include "DescriptorTableD3D11.h"
#include "RootSignatureD3D11.h"

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

  void setDescriptorTables(DescriptorTable** tables, U32 tableCount) override {
    numDescriptorCount = tableCount;
    for (U32 i = 0; i < tableCount; ++i) {
      DescriptorTableD3D11* descTable = static_cast<DescriptorTableD3D11*>(tables[i]);
      _pCurrentDescriptorTableBinds[i] = descTable;
    }
    isDirty = true;
  }

  void setGraphicsRootSignature(RootSignature* pRootSignature) override {
    if (!pRootSignature) return;
    _pCurrentGraphicsRootSignatureBind = static_cast<RootSignatureD3D11*>(pRootSignature);
  }

  void setComputeRootSignature(RootSignature* pRootSignature) override {
    if (!pRootSignature) return;
    _pCurrentComputeRootSignatureBind = static_cast<RootSignatureD3D11*>(pRootSignature);
  }

  void setVertexBuffers(U32 startSlot, 
                        VertexBufferView** ppBuffers,
                        U32 vertexBufferCount) override {
    static U32 strides[16];
    static U32 offsets[16];
    static ID3D11Buffer* buffers[16];

    for (U32 i = 0; i < vertexBufferCount; ++i) {
        VertexBufferViewD3D11* pView = static_cast<VertexBufferViewD3D11*>(ppBuffers[i]);
        ID3D11Buffer* pBuff = static_cast<ID3D11Buffer*>(getBackendD3D11()->getResource(pView->_buffer));
        buffers[i] = pBuff;
        strides[i] = pView->_stride;
        offsets[i] = 0;
    }

    m_ctx->IASetVertexBuffers(startSlot, vertexBufferCount, buffers, strides, offsets );
  }


  void setIndexBuffer(IndexBufferView* pIndexBuffer) override {
    if (!pIndexBuffer) {
      return;
    } 
    IndexBufferViewD3D11* pView = static_cast<IndexBufferViewD3D11*>(pIndexBuffer);
    ID3D11Buffer* pBuffer = static_cast<ID3D11Buffer*>(getBackendD3D11()->getResource(pView->_buffer));
    m_ctx->IASetIndexBuffer(pBuffer, pView->_format, 0);
  }


  void copyResource(Resource* pDst, Resource* pSrc) override {
    if (!pDst || !pSrc) return;
    ID3D11Resource* pNativeSrc = getBackendD3D11()->getResource(pSrc->getUUID());
    ID3D11Resource* pNativeDst = getBackendD3D11()->getResource(pDst->getUUID());
    m_ctx->CopyResource(pNativeDst, pNativeSrc);
  }

  ID3D11DeviceContext* m_ctx;
  D3D11Backend* pBackend;
  ID3D11CommandList* m_pCmdList;
  DescriptorTableD3D11* _pCurrentDescriptorTableBinds[32];
  size_t numDescriptorCount;
  RootSignatureD3D11* _pCurrentGraphicsRootSignatureBind;
  RootSignatureD3D11* _pCurrentComputeRootSignatureBind;
  SIZEB isDirty;
  
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