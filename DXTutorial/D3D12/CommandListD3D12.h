//
#pragma once

#include "../Renderer.h"
#include "CommonsD3D12.h"
#include "D3D12Backend.h"
#include "RenderPassD3D12.h"


namespace gfx {

class GraphicsCommandListD3D12 : public CommandList
{
public:
    GraphicsCommandListD3D12(D3D12Backend* pRenderer, 
                             D3D12_COMMAND_LIST_TYPE type, 
                             ID3D12CommandAllocator** pAllocs, U32 allocCount)
        : m_pAllocatorRef(allocCount)
        , m_type(type)
        , pBackend(pRenderer)
        , CommandList()
    {
      for (U32 i = 0; i < allocCount; ++i)
        m_pAllocatorRef[i] = pAllocs[i];
    }

    ID3D12CommandList* getNativeList(U32 frameIdx) {
      return m_pCmdList[frameIdx];
    }
    
    virtual ~GraphicsCommandListD3D12() { }

    virtual void init() override {
        m_pCmdList.resize(m_pAllocatorRef.size());
        for (U32 i = 0; i < m_pAllocatorRef.size(); ++i) {
          DX12ASSERT(
            pBackend->getDevice()->CreateCommandList(0, 
                                                     m_type, 
                                                     m_pAllocatorRef[i], 
                                                     nullptr, 
                                                     __uuidof(ID3D12GraphicsCommandList), 
                                                     (void**)&m_pCmdList[i]));
        }
    }

    virtual void destroy() override {
        for (U32 i = 0; i < m_pCmdList.size(); ++i)
        m_pCmdList[i]->Release();
    }

    virtual void reset() override {
        U32 frameIndex = pBackend->getFrameIndex();
        m_pCmdList[frameIndex]->Reset(m_pAllocatorRef[frameIndex], nullptr);
    }

    virtual void drawIndexedInstanced(U32 indexCountPerInstance, 
                                      U32 instanceCount, 
                                      U32 startIndexLocation, 
                                      U32 baseVertexLocation, 
                                      U32 startInstanceLocation) override {
        m_pCmdList[pBackend->getFrameIndex()]->DrawIndexedInstanced(indexCountPerInstance, 
                                                                    instanceCount, 
                                                                    startIndexLocation, 
                                                                    baseVertexLocation, 
                                                                    startInstanceLocation);
    }

    virtual void drawInstanced(U32 vertexCountPerInstance, 
                       U32 instanceCount, 
                       U32 startVertexLocation, 
                       U32 startInstanceLocation) override {
        m_pCmdList[pBackend->getFrameIndex()]->DrawInstanced(vertexCountPerInstance, 
                                                             instanceCount, 
                                                             startVertexLocation, 
                                                             startInstanceLocation);
    }

    virtual void setGraphicsPipeline(GraphicsPipeline* pPipeline) override {
      if (!pPipeline) return;

      ID3D12PipelineState* pso = pBackend->getPipelineState(pPipeline->getUUID());
      m_pCmdList[pBackend->getFrameIndex()]->SetPipelineState(pso);
    }

    virtual void setRenderPass(RenderPass* pass) override { 
      if (!pass) { 
        m_pCmdList[pBackend->getFrameIndex()]->OMSetRenderTargets(0,
                                                                  nullptr,
                                                                  FALSE,
                                                                  nullptr);
        return;
      }

      static D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
      static D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
      RenderPassD3D12* nativePass = static_cast<RenderPassD3D12*>(pass);
      U32 rtvCount = static_cast<U32>(nativePass->_renderTargetResourceIds.size());

      if (nativePass != pBackend->getBackbufferRenderPass()) {
        for (U32 i = 0; i < rtvCount; ++i) {
          rtvHandles[i] = pBackend->getViewHandle(nativePass->_renderTargetResourceIds[i]);
        }

        if (nativePass->_depthStencilResourceId) {
          dsvHandle = pBackend->getViewHandle(nativePass->_depthStencilResourceId); 
        } else {
          dsvHandle = { 0 };
        }
      
        m_pCmdList[pBackend->getFrameIndex()]->OMSetRenderTargets(rtvCount,
                                       rtvHandles,
                                       FALSE,
                                       &dsvHandle);
      } else {
        rtvCount = 1;
        m_pCmdList[pBackend->getFrameIndex()]->OMSetRenderTargets(
            rtvCount,
            &pBackend->getViewHandle(nativePass->_renderTargetResourceIds[pBackend->getFrameIndex()]),
            FALSE, nullptr);
      }
    }

    virtual void dispatch(U32 x, U32 y, U32 z) override {
        m_pCmdList[pBackend->getFrameIndex()]->Dispatch(x, y, z);
    }

    virtual void setVertexBuffers(VertexBufferView** buffers,
                                  U32 vertexBufferCount) override {
        static D3D12_VERTEX_BUFFER_VIEW* kVertexBuffers[16];
        //m_pCmdList->IASetVertexBuffers
        for (U32 i = 0; i < vertexBufferCount; ++i) {
          
        }

        m_pCmdList[pBackend->getFrameIndex()];
    }

    virtual void setIndexBuffer(IndexBufferView* buffer) override {}

    virtual void close() override {
        m_pCmdList[pBackend->getFrameIndex()]->Close();
    }

    virtual void setViewports(Viewport* pViewports, U32 viewportCount) override {  
        static D3D12_VIEWPORT kNativeViewports[16];
        for (U32 i = 0; i < viewportCount; ++i) {
            Viewport& vp = pViewports[i];
            kNativeViewports[i] = { vp.x, 
                                    vp.y,
                                    vp.w,
                                    vp.h,
                                    vp.mind,
                                    vp.maxd };
        }

        m_pCmdList[pBackend->getFrameIndex()]->RSSetViewports(viewportCount, kNativeViewports);
    }

    virtual void setScissors(Scissor* pScissors, U32 scissorCount) override {
        static D3D12_RECT kNativeScissors[32];
        for (U32 i = 0; i < scissorCount; ++i) {
            Scissor& sr = pScissors[i];
            kNativeScissors[i] = {  static_cast<LONG>(sr.left),
                                    static_cast<LONG>(sr.top),
                                    static_cast<LONG>(sr.right),
                                    static_cast<LONG>(sr.bottom) };
        }

        m_pCmdList[pBackend->getFrameIndex()]->RSSetScissorRects(scissorCount, kNativeScissors);
    }

    virtual void setDescriptorTables(DescriptorTable** pTables,
                                     U32 tableCount) override {}

    virtual void clearRenderTarget(RenderTargetView* view, 
                           R32* rgba, 
                           U32 numRects,
                           RECT* rects) override {
        m_pCmdList[pBackend->getFrameIndex()]->ClearRenderTargetView(pBackend->getViewHandle(view->getUUID()),
                                                                     rgba,
                                                                     numRects,
                                                                     rects);
    }

protected:
   std::vector<ID3D12GraphicsCommandList*> m_pCmdList;
    std::vector<ID3D12CommandAllocator*> m_pAllocatorRef;
    D3D12Backend* pBackend;
    D3D12_COMMAND_LIST_TYPE m_type;
};


class StaticGraphicsCommandListD3D12 : public GraphicsCommandListD3D12
{
public:
    StaticGraphicsCommandListD3D12(D3D12Backend* pRenderer, 
                                      D3D12_COMMAND_LIST_TYPE type, 
                                      ID3D12CommandAllocator** pAllocs, U32 allocCount)
        : GraphicsCommandListD3D12(pRenderer, type, pAllocs, allocCount)
    {
    }

    ~StaticGraphicsCommandListD3D12() { }

    void init() override {
        m_pCmdList.resize(m_pAllocatorRef.size());
        for (U32 i = 0; i < m_pAllocatorRef.size(); ++i) {
          DX12ASSERT(
            pBackend->getDevice()->CreateCommandList(0, 
                                                     m_type, 
                                                     m_pAllocatorRef[i], 
                                                     nullptr, 
                                                     __uuidof(ID3D12GraphicsCommandList), 
                                                     (void**)&m_pCmdList[i]));
        }
    }

    void destroy() override {
        for (U32 i = 0; i < m_pCmdList.size(); ++i)
          m_pCmdList[i]->Release();
    }

    void reset() override {
        for (U32 i = 0; i < m_pCmdList.size(); ++i)
          m_pCmdList[i]->Reset(m_pAllocatorRef[i], nullptr);
    }

    void drawIndexedInstanced(U32 indexCountPerInstance, 
                              U32 instanceCount, 
                              U32 startIndexLocation, 
                              U32 baseVertexLocation, 
                              U32 startInstanceLocation) override {
        for (U32 i = 0; i < m_pCmdList.size(); ++i)
          m_pCmdList[i]->DrawIndexedInstanced(indexCountPerInstance, 
                                              instanceCount, 
                                              startIndexLocation, 
                                              baseVertexLocation, 
                                              startInstanceLocation);
    }

    void drawInstanced(U32 vertexCountPerInstance, 
                       U32 instanceCount, 
                       U32 startVertexLocation, 
                       U32 startInstanceLocation) override {
        for (U32 i = 0; i < m_pCmdList.size(); ++i)
          m_pCmdList[i]->DrawInstanced(vertexCountPerInstance, 
                                       instanceCount, 
                                       startVertexLocation, 
                                       startInstanceLocation);
    }

    void setGraphicsPipeline(GraphicsPipeline* pPipeline) override {
      ID3D12PipelineState* pso = pBackend->getPipelineState(pPipeline->getUUID());
      for (U32 i = 0; i < m_pCmdList.size(); ++i)
        m_pCmdList[i]->SetPipelineState(pso);
    }

    void setRenderPass(RenderPass* pass) override { 
      if (!pass) {
        for (U32 i = 0; i < m_pCmdList.size(); ++i)
          m_pCmdList[i]->OMSetRenderTargets(0,
                                            nullptr,
                                            FALSE,
                                            nullptr);
        return;
      }

      static D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
      static D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
      U32 rtvCount = 0;
      RenderPassD3D12* nativePass = static_cast<RenderPassD3D12*>(pass);

      if (nativePass != pBackend->getBackbufferRenderPass()) {
        rtvCount = static_cast<U32>(nativePass->_renderTargetResourceIds.size());

        for (U32 i = 0; i < rtvCount; ++i) {
          rtvHandles[i] = pBackend->getViewHandle(nativePass->_renderTargetResourceIds[i]);
        }

        if (nativePass->_depthStencilResourceId) {
          dsvHandle = pBackend->getViewHandle(nativePass->_depthStencilResourceId); 
        } else {
          dsvHandle = { 0 };
        }

        for (U32 i = 0; i < m_pCmdList.size(); ++i)
          m_pCmdList[i]->OMSetRenderTargets(rtvCount,
                                            rtvHandles,
                                            FALSE,
                                            &dsvHandle);
      } else {
        for (U32 i = 0; i < m_pCmdList.size(); ++i) {
          rtvCount = 1;
          m_pCmdList[i]->OMSetRenderTargets(rtvCount,
                                            &pBackend->getViewHandle(nativePass->_renderTargetResourceIds[i]),
                                            FALSE,
                                            nullptr);
        }
      }
    }

    void dispatch(U32 x, U32 y, U32 z) override {
        for (U32 i = 0; i < m_pCmdList.size(); ++i)
          m_pCmdList[i]->Dispatch(x, y, z);
    }

    void setVertexBuffers(VertexBufferView** buffers, U32 vertexBufferCount) override {
        //m_pCmdList->IASetVertexBuffers
        
    }

    void setIndexBuffer(IndexBufferView* buffer) override { }

    void close() override {
      for (U32 i = 0; i < m_pCmdList.size(); ++i)
        m_pCmdList[i]->Close();
    }

    void setViewports(Viewport* pViewports, U32 viewportCount) override {  
        static D3D12_VIEWPORT kNativeViewports[16];
        for (U32 i = 0; i < viewportCount; ++i) {
            Viewport& vp = pViewports[i];
            kNativeViewports[i] = { vp.x, 
                                    vp.y,
                                    vp.w,
                                    vp.h,
                                    vp.mind,
                                    vp.maxd };
        }

        for (U32 i = 0; i < m_pCmdList.size(); ++i) 
          m_pCmdList[i]->RSSetViewports(viewportCount, kNativeViewports);
    }

    void setScissors(Scissor* pScissors, U32 scissorCount) override {
        static D3D12_RECT kNativeScissors[32];
        for (U32 i = 0; i < scissorCount; ++i) {
            Scissor& sr = pScissors[i];
            kNativeScissors[i] = {  static_cast<LONG>(sr.left),
                                    static_cast<LONG>(sr.top),
                                    static_cast<LONG>(sr.right),
                                    static_cast<LONG>(sr.bottom) };
        }

      for (U32 i = 0; i < m_pCmdList.size(); ++i)
        m_pCmdList[i]->RSSetScissorRects(scissorCount, kNativeScissors);
    }

    void setDescriptorTables(DescriptorTable** pTables, U32 tableCount) override { }

    void clearRenderTarget(RenderTargetView* view, R32* rgba, U32 numRects, RECT* rects) override {
      if (view == pBackend->getSwapchainRenderTargetView()) {
        RenderPassD3D12* rp = static_cast<RenderPassD3D12*>(pBackend->getBackbufferRenderPass());
        for (U32 i = 0; i < m_pCmdList.size(); ++i) {
          m_pCmdList[i]->ClearRenderTargetView(pBackend->getViewHandle(rp->_renderTargetResourceIds[i]),
                                               rgba, 
                                               numRects, 
                                               rects);
        }
      } else {
        for (U32 i = 0; i < m_pCmdList.size(); ++i) {
          m_pCmdList[i]->ClearRenderTargetView(
              pBackend->getViewHandle(view->getUUID()), 
                                      rgba,
                                      numRects, 
                                      rects);
        }
      }
    }
};
} // gfx