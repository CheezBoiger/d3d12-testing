//
#pragma once

#include "../BackendRenderer.h"
#include "CommonsD3D12.h"
#include "D3D12Backend.h"
#include "RenderPassD3D12.h"
#include "PipelineStatesD3D12.h"
#include <pix.h>

namespace gfx {


D3D12_CLEAR_FLAGS getDepthClearFlags(ClearFlags flags)
{
  D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAGS(0);
  if (flags & CLEAR_FLAG_DEPTH)
    clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
  if (flags & CLEAR_FLAG_STENCIL)
    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
  return clearFlags;
}

class GraphicsCommandListD3D12 : public CommandList
{
public:

    GraphicsCommandListD3D12
        (
            D3D12_COMMAND_LIST_TYPE type, 
            ID3D12CommandAllocator** pAllocs, 
            U32 allocCount
        )
            : m_pAllocatorRef(allocCount)
            , m_type(type)
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
            getBackendD3D12()->getDevice()->CreateCommandList(0, 
                                                     m_type, 
                                                     m_pAllocatorRef[i], 
                                                     nullptr, 
                                                     __uuidof(ID3D12GraphicsCommandList), 
                                                     (void**)&m_pCmdList[i]));
          m_pCmdList[i]->Close();
          _isRecording = false;
        }
    }

    virtual void destroy() override {
        for (U32 i = 0; i < m_pCmdList.size(); ++i)
        m_pCmdList[i]->Release();
    }

    virtual void reset(const char* debugTag) override {
        
        U32 frameIndex = getBackendD3D12()->getFrameIndex();
        const char* tag = debugTag;

        if (!debugTag)
            tag = "";
        m_pCmdList[frameIndex]->Reset(m_pAllocatorRef[frameIndex], nullptr);
        PIXBeginEvent(m_pCmdList[frameIndex], 0, tag);
        _isRecording = true;
    }

    virtual void drawIndexedInstanced
        (
            U32 indexCountPerInstance, 
            U32 instanceCount, 
            U32 startIndexLocation, 
            U32 baseVertexLocation, 
            U32 startInstanceLocation
        ) override 
    {
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->DrawIndexedInstanced(indexCountPerInstance, 
                                                                            instanceCount, 
                                                                            startIndexLocation, 
                                                                            baseVertexLocation, 
                                                                            startInstanceLocation);
    }

    void setMarker(const char* tag) override {
        const char* t = tag;

        if (!tag) t = "";

        PIXSetMarker(m_pCmdList[getBackendD3D12()->getFrameIndex()], 0, t);
    }

    virtual void drawInstanced
        (
            U32 vertexCountPerInstance, 
            U32 instanceCount, 
            U32 startVertexLocation, 
            U32 startInstanceLocation
        ) override 
    {
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->DrawInstanced(vertexCountPerInstance, 
                                                                     instanceCount, 
                                                                     startVertexLocation, 
                                                                     startInstanceLocation);
    }

    virtual void setGraphicsPipeline(GraphicsPipeline* pPipeline) override {
      if (!pPipeline) return;

      ID3D12PipelineState* pso = getBackendD3D12()->getPipelineState(pPipeline->getUUID());
      GraphicsPipelineStateD3D12* pGraphicsPipeline = static_cast<GraphicsPipelineStateD3D12*>(pPipeline);
      m_pCmdList[getBackendD3D12()->getFrameIndex()]->IASetPrimitiveTopology(pGraphicsPipeline->_topology);
      m_pCmdList[getBackendD3D12()->getFrameIndex()]->SetPipelineState(pso);
    }


    virtual void setComputePipeline(ComputePipeline* pPipeline) override {
        if (!pPipeline) return;
        ID3D12PipelineState* pso = getBackendD3D12()->getPipelineState(pPipeline->getUUID());
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->SetPipelineState(pso); 
    }

    virtual void setRenderPass(RenderPass* pass) override { 
      if (!pass) { 
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->OMSetRenderTargets(
            0,
                                                                  nullptr,
                                                                  FALSE,
                                                                  nullptr);
        return;
      }

      static D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
      static D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
      RenderPassD3D12* nativePass = static_cast<RenderPassD3D12*>(pass);
      U32 rtvCount = static_cast<U32>(nativePass->_renderTargetViews.size());

      if (nativePass != getBackendD3D12()->getBackbufferRenderPass()) {
        for (U32 i = 0; i < rtvCount; ++i) {
          rtvHandles[i] = getBackendD3D12()->getViewHandle(nativePass->_renderTargetViews[i]->getUUID());
        }

        if (nativePass->_depthStencilResourceId) {
          dsvHandle = getBackendD3D12()->getViewHandle(nativePass->_depthStencilResourceId->getUUID()); 
        } else {
          dsvHandle = { 0 };
        }
      
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->OMSetRenderTargets(rtvCount,
                                       rtvHandles,
                                       FALSE,
                                       &dsvHandle);
      } else {
        rtvCount = 1;
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->OMSetRenderTargets(
            rtvCount,
            &getBackendD3D12()->getViewHandle(nativePass->_renderTargetViews[getBackendD3D12()->getFrameIndex()]->getUUID()),
            FALSE, nullptr);
      }
    }

    virtual void dispatch(U32 x, U32 y, U32 z) override {
      m_pCmdList[getBackendD3D12()->getFrameIndex()]->Dispatch(x, y, z);
    }

    virtual void setVertexBuffers(U32 startSlot,
                                  VertexBufferView** buffers,
                                  U32 vertexBufferCount) override {
        static D3D12_VERTEX_BUFFER_VIEW kVertexBuffers[16];
        //m_pCmdList->IASetVertexBuffers
        for (U32 i = 0; i < vertexBufferCount; ++i) {
          VertexBufferViewD3D12* pView = static_cast<VertexBufferViewD3D12*>(buffers[i]);
          kVertexBuffers[i].BufferLocation = getBackendD3D12()->getResource(pView->_buffer)->GetGPUVirtualAddress();
          kVertexBuffers[i].SizeInBytes = pView->_szInBytes;
          kVertexBuffers[i].StrideInBytes = pView->_vertexStrideBytes;
        }

        m_pCmdList[getBackendD3D12()->getFrameIndex()]->IASetVertexBuffers(startSlot, 
                                                                           vertexBufferCount, 
                                                                           kVertexBuffers);
    }

    virtual void setIndexBuffer(IndexBufferView* buffer) override {
      if (!buffer) return;
      static D3D12_INDEX_BUFFER_VIEW kIndexBuffer;
      IndexBufferViewD3D12* pView = static_cast<IndexBufferViewD3D12*>(buffer);
      kIndexBuffer.BufferLocation = getBackendD3D12()->getResource(pView->_buffer)->GetGPUVirtualAddress();
      kIndexBuffer.Format = pView->_format;
      kIndexBuffer.SizeInBytes = pView->_szBytes;
      m_pCmdList[getBackendD3D12()->getFrameIndex()]->IASetIndexBuffer(&kIndexBuffer);
    }

    virtual void close() override {
        PIXEndEvent();
      m_pCmdList[getBackendD3D12()->getFrameIndex()]->Close();
        _isRecording = false;
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

        m_pCmdList[getBackendD3D12()->getFrameIndex()]->RSSetViewports(
            viewportCount, kNativeViewports);
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

        m_pCmdList[getBackendD3D12()->getFrameIndex()]->RSSetScissorRects(
            scissorCount, kNativeScissors);
    }

    virtual void clearRenderTarget(RenderTargetView* view, 
                           R32* rgba, 
                           U32 numRects,
                           RECT* rects) override {
      ViewHandleD3D12* pNativeView = static_cast<ViewHandleD3D12*>(view); 
        if (pNativeView->_currentState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
          D3D12_RESOURCE_BARRIER barrier = {};
          barrier.Transition.pResource =
              getBackendD3D12()->getResource(pNativeView->_buffer);
          barrier.Transition.StateBefore = pNativeView->_currentState;
          barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
          barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
          barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; 
          barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
          m_pCmdList[getBackendD3D12()->getFrameIndex()]->ResourceBarrier(
              1, &barrier);
          pNativeView->_currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
        }

        m_pCmdList[getBackendD3D12()->getFrameIndex()]->ClearRenderTargetView(
            getBackendD3D12()->getViewHandle(view->getUUID()),
                                                                     rgba,
                                                                     numRects,
                                                                     rects);
    }

    virtual void clearDepthStencil(DepthStencilView* view, 
                                   ClearFlags flags, 
                                   R32 depth, 
                                   U8 stencil, 
                                   U32 numRects, 
                                   const RECT* rects) override {
      U32 frameIdx = getBackendD3D12()->getFrameIndex();
      ViewHandleD3D12* pView = static_cast<ViewHandleD3D12*>(view);
      ID3D12Resource* pResource = getBackendD3D12()->getResource(pView->_buffer);
      D3D12_CPU_DESCRIPTOR_HANDLE handle = getBackendD3D12()->getViewHandle(view->getUUID());
      D3D12_CLEAR_FLAGS clearFlags = getDepthClearFlags(flags);
      m_pCmdList[frameIdx]->ClearDepthStencilView(handle, 
                                                  clearFlags, 
                                                  depth, 
                                                  stencil, 
                                                  numRects, 
                                                  rects);
    }


    virtual void setDescriptorTables(DescriptorTable** tables, U32 tableCount) override {
      U32 frameIdx = getBackendD3D12()->getFrameIndex();
      if (!tables || !tableCount) {
        m_pCmdList[frameIdx]->SetDescriptorHeaps(0, nullptr);
      }

      static ID3D12DescriptorHeap* pHeaps[32];
      for (U32 i = 0; i < tableCount; ++i)
        pHeaps[i] = getBackendD3D12()->getDescriptorHeap(tables[i]->getUUID());

      m_pCmdList[frameIdx]->SetDescriptorHeaps(tableCount, pHeaps);
    }

    virtual void setGraphicsRootSignature(RootSignature* pRootSignature) override {
      U32 frameIdx = getBackendD3D12()->getFrameIndex();
      if (!pRootSignature) {
        m_pCmdList[frameIdx]->SetGraphicsRootSignature(nullptr);
      }

      ID3D12RootSignature* pNativeRootSig =
          getBackendD3D12()->getRootSignature(pRootSignature->getUUID());
      m_pCmdList[frameIdx]->SetGraphicsRootSignature(pNativeRootSig);
    }

    virtual void setComputeRootSignature(RootSignature* pRootSignature) override {
      U32 frameIdx = getBackendD3D12()->getFrameIndex();
      if (!pRootSignature) {
        m_pCmdList[frameIdx]->SetComputeRootSignature(nullptr);
      }

      ID3D12RootSignature* pNativeRootSig =
          getBackendD3D12()->getRootSignature(pRootSignature->getUUID());
      m_pCmdList[frameIdx]->SetComputeRootSignature(pNativeRootSig);
    }

    void copyResource(Resource* pDst, Resource* pSrc) override {
      if (!pSrc || !pDst) return;

      ID3D12Resource* pNativeSrc = getBackendD3D12()->getResource(pSrc->getUUID());
      ID3D12Resource* pNativeDst = getBackendD3D12()->getResource(pDst->getUUID());
      m_pCmdList[getBackendD3D12()->getFrameIndex()]->CopyResource(pNativeDst, pNativeSrc);  
    }

    void setGraphicsRootDescriptorTable(U32 rootParameterIndex, DescriptorTable* pTable) override {
        if (!pTable) return;
        ID3D12DescriptorHeap* pHeap = getBackendD3D12()->getDescriptorHeap(pTable->getUUID());
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->SetGraphicsRootDescriptorTable(rootParameterIndex, 
                                                                                       pHeap->GetGPUDescriptorHandleForHeapStart());
    }


    virtual void setComputeRootDescriptorTable(U32 rootParameterIndex, DescriptorTable* pTable) override { 
        if (!pTable) return;
        ID3D12DescriptorHeap* pHeap = getBackendD3D12()->getDescriptorHeap(pTable->getUUID());
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->SetComputeRootDescriptorTable(rootParameterIndex,
                                                                                      pHeap->GetGPUDescriptorHandleForHeapStart());
    }

    void setGraphicsRootConstantBufferView(U32 rootParameterIndex, Resource* pConstantBuffer, U64 offset) override {
      ID3D12Resource* pResource = getBackendD3D12()->getResource(pConstantBuffer->getUUID());
      m_pCmdList[getBackendD3D12()->getFrameIndex()]->SetGraphicsRootConstantBufferView(rootParameterIndex, 
                                                                                        pResource->GetGPUVirtualAddress() + offset);
    }

    void setComputeRootConstantBufferView(U32 rootParameterIndex, Resource* pConstantBuffer, U64 offset) override {
      ID3D12Resource* pResource = getBackendD3D12()->getResource(pConstantBuffer->getUUID());
      m_pCmdList[getBackendD3D12()->getFrameIndex()]->SetComputeRootConstantBufferView(rootParameterIndex, 
                                                                                        pResource->GetGPUVirtualAddress() + offset);
    }

    void setGraphicsRootShaderResourceView(U32 rootParameterIndex, Resource* pShaderResourceView) override {
        ID3D12Resource* pResource = getBackendD3D12()->getResource(pShaderResourceView->getUUID());
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->SetGraphicsRootShaderResourceView(rootParameterIndex,
                                                                                          pResource->GetGPUVirtualAddress());
    }

    void setComputeRootShaderResourceView(U32 rootParameterIndex, Resource* pShaderResourceView) override {
        ID3D12Resource* pResource = getBackendD3D12()->getResource(pShaderResourceView->getUUID());
        m_pCmdList[getBackendD3D12()->getFrameIndex()]->SetComputeRootShaderResourceView(rootParameterIndex,
                                                                                         pResource->GetGPUVirtualAddress());
    }

protected:
   std::vector<ID3D12GraphicsCommandList*> m_pCmdList;
    std::vector<ID3D12CommandAllocator*> m_pAllocatorRef;
    D3D12_COMMAND_LIST_TYPE m_type;
};

#if 0
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
          m_pCmdList[i]->Close();
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
      if (!pPipeline) {
        return;
      }
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
        rtvCount = static_cast<U32>(nativePass->_renderTargetViews.size());

        for (U32 i = 0; i < rtvCount; ++i) {
          rtvHandles[i] = pBackend->getViewHandle(nativePass->_renderTargetViews[i]->getUUID());
        }

        if (nativePass->_depthStencilResourceId) {
          dsvHandle = pBackend->getViewHandle(nativePass->_depthStencilResourceId->getUUID()); 
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
                                            &pBackend->getViewHandle(nativePass->_renderTargetViews[i]->getUUID()),
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
        DX12ASSERT(m_pCmdList[i]->Close());
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
          ViewHandleD3D12* pNativeView = static_cast<ViewHandleD3D12*>(rp->_renderTargetViews[i]); 
          if (pNativeView->_currentState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Transition.pResource = pBackend->getResource(pNativeView->getUUID());
            barrier.Transition.StateBefore = pNativeView->_currentState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; 
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            m_pCmdList[i]->ResourceBarrier(1, &barrier);
            pNativeView->_currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
          }
          m_pCmdList[i]->ClearRenderTargetView(pBackend->getViewHandle(rp->_renderTargetViews[i]->getUUID()),
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
}
#endif
} // gfx