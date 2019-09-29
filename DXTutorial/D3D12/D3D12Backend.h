
#pragma once

#include "CommonsD3D12.h"
#include "MemoryAllocatorD3D12.h"
#include "RenderPassD3D12.h"
#include "../Renderer.h"

#include <vector>
#include <unordered_map>

namespace gfx 
{

class D3D12Backend;

// Frame Resources.
struct FrameResource
{
    ID3D12CommandAllocator* _pAllocator;
    ID3D12Resource* _swapImage;
    Fence _pSignalFence;
    HANDLE _signalEvent;
    RenderTargetView _rtv;
};


enum DescriptorHeapType 
{
  DESCRIPTOR_HEAP_START = 0,

  DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS = DESCRIPTOR_HEAP_START,
  DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS,
  DESCRIPTOR_HEAP_SHADER_RESOURCE_VIEWS,
  DESCRIPTOR_HEAP_CONSTANT_BUFFER_VIEWS,
  DESCRIPTOR_HEAP_UNORDERD_ACCESS_VIEWS,
  DESCRIPTOR_HEAP_SAMPLER,

  DESCRIPTOR_HEAP_END
};

typedef U32 DescriptorHeapT;

struct BufferD3D12 : public Buffer
{
  BufferD3D12(D3D12Backend* backend,
              BufferUsage usage,
              U32 structureByteStride)
    : pBackend(backend)
    , _structureByteStride(structureByteStride)
    , _usage(usage) { }
  void* map(U64 start, U64 sz) override;
  void unmap(U64 start, U64 sz) override;
  BufferUsage _usage;
  D3D12Backend* pBackend;
  U32 _structureByteStride;
};

struct ViewHandleD3D12 : public TargetView
{
};


struct VertexBufferViewD3D12 : public TargetView
{
  RendererT _buffer;
  U32 _szInBytes;
  U32 _vertexStrideBytes;
};


class D3D12Backend : public BackendRenderer
{
public:
    static const U64 kGraphicsQueueId = 0;

    D3D12Backend();

    void initialize(HWND handle, 
                    bool isFullScreen, 
                    const GpuConfiguration& configs) override;
    void cleanUp() override { }

    void present() override;
    void submit(RendererT queue, CommandList** cmdLists, U32 numCmdLists) override;
    void signalFence(RendererT queue, Fence* fence) override;
    void waitFence(Fence* fence) override;

    void createCommandList(CommandList** pList,
                           CommandListRecordUsage usage) override;
    
    void createRenderPass(RenderPass** pass,
                                  U32 rtvSize, 
                                  B32 hasDepthStencil) override;
    void destroyRenderPass(RenderPass* pass) override;
    void createBuffer(Buffer** buffer, 
                      BufferUsage usage,
                      BufferBindFlags binds, 
                      BufferDimension dimension, 
                      U32 width, 
                      U32 height = 1,
                      U32 depth = 1,
                      U32 structureByteStride = 1,
                      DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN,
                      const TCHAR* debugName = nullptr) override;
    void destroyBuffer(Buffer* buffer) override;
    void createRenderTargetView(RenderTargetView** rtv, Buffer* buffer) override;
    void createUnorderedAccessView(UnorderedAccessView** uav, Buffer* buffer) override;
    void createShaderResourceView(ShaderResourceView** srv, 
                                  Buffer* buffer, 
                                  U32 firstElement, 
                                  U32 numElements) override;

    virtual void createVertexBufferView(VertexBufferView** view,
                                        Buffer* buffer, 
                                        U32 vertexStride, 
                                        U32 bufferSzBytes) override { }
    virtual void createIndexBufferView(IndexBufferView** view) override { }

    void createDepthStencilView(DepthStencilView** dsv, Buffer* buffer) override;
    void destroyCommandList(CommandList* pList) override;

    ID3D12Resource* getResource(RendererT uuid) { 
      return m_resources[uuid];
    }


    ID3D12Device* getDevice() { 
      return m_pDevice;
    }

    ID3D12PipelineState* getPipelineState(RendererT uuid) {
      return m_pPipelineStates[uuid];
    }

    ID3D12DescriptorHeap* getDescriptorHeap(DescriptorHeapT uuid) {
      return m_pDescriptorHeaps[uuid];
    }

    D3D12_CPU_DESCRIPTOR_HANDLE getViewHandle(RendererT uuid) {
      return m_viewHandles[uuid];
    }

    U32 getFrameIndex() const { return m_frameIndex; }

    RenderPass* getBackbufferRenderPass() override { return m_pSwapchainPass; }

    RenderTargetView* getSwapchainRenderTargetVew() override { 
      return &m_frameResources[m_frameIndex]._rtv;
    }

    RendererT getSwapchainQueue() override { return kGraphicsQueueId; }
    Fence* getSwapchainFence() override { return &m_frameResources[m_frameIndex]._pSignalFence; }

private:

    void queryForDevice(IDXGIFactory4* pFactory);
    void createSwapChain(IDXGIFactory4* pFactory,
                         HWND handle, 
                         U32 renderWidth, 
                         U32 renderHeight, 
                         U32 desiredBuffers, 
                         B32 windowed);
    void querySwapChain();
    void createGraphicsQueue();
    void createHeaps();
    void createDescriptorHeaps();
    IDXGIFactory4* createFactory();
    void createCommandAllocators() { }

    std::unordered_map<RendererT, CommandList*> m_cmdLists;
    std::unordered_map<RendererT, ID3D12Resource*> m_resources;
    std::unordered_map<DescriptorHeapT, ID3D12Heap*> m_pHeaps;
    std::unordered_map<DescriptorHeapT, ID3D12DescriptorHeap*> m_pDescriptorHeaps;
    std::unordered_map<RendererT, ID3D12RootSignature*> m_pRootSignatures;
    std::unordered_map<RendererT, ID3D12CommandQueue*> m_pCommandQueues;
    std::unordered_map<RendererT, ID3D12CommandAllocator*> m_pCommandAllocators;
    std::unordered_map<RendererT, ID3D12PipelineState*> m_pPipelineStates;
    std::unordered_map<RendererT, ID3D12Fence*> m_fences;
    std::unordered_map<RendererT, HANDLE> m_fenceEvents;
    std::unordered_map<RendererT, U64> m_fenceValues;
    std::unordered_map<RendererT, D3D12_CPU_DESCRIPTOR_HANDLE> m_viewHandles;
    std::unordered_map<DescriptorHeapT, D3D12_CPU_DESCRIPTOR_HANDLE> m_descriptorHeapCurrentOffset; 
    std::unordered_map<RendererT, D3D12_VERTEX_BUFFER_VIEW> m_vertexBufferViews;
    std::unordered_map<RendererT, D3D12_INDEX_BUFFER_VIEW> m_indexBufferViews;

    MemoryAllocatorD3D12 m_memAllocator;
    ID3D12Device* m_pDevice;
    IDXGISwapChain3* m_pD3D12Swapchain;
    DXGI_SWAP_CHAIN_DESC1 m_swapchainDesc;
    RenderPassD3D12* m_pSwapchainPass;
    std::vector<FrameResource> m_frameResources; 
    U32 m_frameIndex;
};
} // gfx