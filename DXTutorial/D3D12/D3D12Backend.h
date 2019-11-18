
#pragma once

#include "CommonsD3D12.h"
#include "MemoryAllocatorD3D12.h"
#include "D3D12MemAlloc.h"
#include "RenderPassD3D12.h"
#include "../BackendRenderer.h"

#include <vector>
#include <unordered_map>

namespace gfx 
{

class D3D12Backend;

struct ViewHandleD3D12 : public TargetView
{
  D3D12_RESOURCE_STATES _currentState;
  RendererT _buffer;
};


// Frame Resources.
struct FrameResource
{
    static const RendererT kFrameResourceId = 0xffffffffffffffffull;
    // This allocator resets often.
    ID3D12CommandAllocator* _pAllocator;
    ID3D12GraphicsCommandList* _cmdList;
    ID3D12Resource* _swapImage;
    U32 _fenceValue;
    ViewHandleD3D12 _rtv;
};


enum DescriptorHeapType 
{
  DESCRIPTOR_HEAP_START = 0,

  DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS = DESCRIPTOR_HEAP_START,
  DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS,
  DESCRIPTOR_HEAP_SRV_UAV_CBV,
  DESCRIPTOR_HEAP_SAMPLER,

  DESCRIPTOR_HEAP_END
};

typedef RendererT DescriptorHeapT;

struct BufferD3D12 : public Resource
{
  BufferD3D12(D3D12Backend* backend,
              ResourceDimension dimension,
              ResourceUsage usage,
              ResourceBindFlags flags,
              U32 structureByteStride)
    : pBackend(backend)
    , _structureByteStride(structureByteStride)
    , Resource(dimension, usage, flags) { }

  void* map(U64 start, U64 sz) override;
  void unmap(U64 start, U64 sz) override;

  D3D12Backend* pBackend;
  U32 _structureByteStride;
  D3D12_RESOURCE_STATES _currentResourceState;
  D3D12MA::Allocation* pAllocation;
};


struct VertexBufferViewD3D12 : public TargetView
{
  RendererT _buffer;
  U32 _szInBytes;
  U32 _vertexStrideBytes;
};


struct IndexBufferViewD3D12 : public TargetView
{
  RendererT _buffer;
  DXGI_FORMAT _format;
  U32 _szBytes;
};


class D3D12Backend : public BackendRenderer
{
public:

    D3D12Backend();

    void initialize(HWND handle, 
                    bool isFullScreen, 
                    const GpuConfiguration& configs) override;
    void cleanUp() override { }

    void present() override;
    void submit(RendererT queue, CommandList** cmdLists, U32 numCmdLists) override;
    void signalFence(RendererT queue, Fence* fence) override;
    void waitFence(Fence* fence) override;

    void createCommandList(CommandList** pList) override;
    
    void createRenderPass(RenderPass** pass,
                                  U32 rtvSize, 
                                  B32 hasDepthStencil) override;
    void destroyRenderPass(RenderPass* pass) override;
    void createBuffer(Resource** buffer, 
                            ResourceUsage usage,
                            ResourceBindFlags binds, 
                            U32 widthBytes,
                            U32 structureByteStride,
                            const TCHAR* debugName) override;
    void createTexture(Resource** texture,
                       ResourceDimension dimension,
                       ResourceUsage usage,
                       ResourceBindFlags binds,
                       DXGI_FORMAT format,
                       U32 width,
                       U32 height,
                       U32 depth,
                       U32 structureByteStride,
                       const TCHAR* debugName = nullptr) override;
    void destroyResource(Resource* resource) override;
    void createRenderTargetView(RenderTargetView** rtv, Resource* buffer) override;
    void createUnorderedAccessView(UnorderedAccessView** uav, Resource* buffer) override;
    void createShaderResourceView(ShaderResourceView** srv, 
                                  Resource* buffer, 
                                  U32 firstElement, 
                                  U32 numElements) override;

    void createVertexBufferView(VertexBufferView** view,
                                        Resource* buffer, 
                                        U32 vertexStride, 
                                        U32 bufferSzBytes) override;
    void createIndexBufferView(IndexBufferView** view,
                               Resource* buffer,
                               DXGI_FORMAT format,
                               U32 szBytes) override;
    void createRootSignature(RootSignature** pRootSignature) override;
    void destroyRootSignature(RootSignature* pRootSig) override { }

    void createDepthStencilView(DepthStencilView** dsv, Resource* buffer) override;
    void destroyCommandList(CommandList* pList) override;
    void createRayTracingPipelineState(RayTracingPipeline** ppPipeline, 
                                       const RayTracingPipelineInfo* pInfo) override;

    void createSampler(Sampler** sampler, const SamplerDesc* pDesc) override;
    void destroySampler(Sampler* sampler) override { }
    void createDescriptorTable(DescriptorTable** table) override;
    void destroyDescriptorTable(DescriptorTable* table) override { }
    void createFence(Fence** ppFence) override;
    void destroyFence(Fence* pFence) override;
    void createGraphicsPipelineState(GraphicsPipeline** ppPipeline,
                                     const GraphicsPipelineInfo* pInfo) override;
    void createComputePipelineState(ComputePipeline** ppPipeline,
                                    const ComputePipelineInfo* pInfo) override;
    void createAccelerationStructure(Resource** ppResource,
                                     const AccelerationStructureGeometry* geometryInfos, 
                                     U32 geometryCount,
                                     const AccelerationStructureTopLevelInfo* pTopLevelInfo) override;

    ID3D12Resource* getResource(RendererT uuid, size_t resourceIdx = 0xffffffffffffffffull) {
      size_t resourceMax = m_resources[uuid].size(); 
      return m_resources[uuid][ (resourceIdx == 0xffffffffffffffffull ? m_frameIndex : resourceIdx) % resourceMax];
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

    D3D12_CPU_DESCRIPTOR_HANDLE getViewHandle(RendererT uuid, size_t resourceIdx = 0xffffffffffffffffull) {
      size_t viewMax = m_viewHandles[uuid].size();
      return m_viewHandles[uuid][ (resourceIdx == 0xffffffffffffffffull ? m_frameIndex : resourceIdx) % viewMax];
    }

    ID3D12RootSignature* getRootSignature(RendererT uuid) {
      return m_pRootSignatures[uuid];
    }
    
    void setRootSignature(RendererT uuid, ID3D12RootSignature* rootSig) {
      m_pRootSignatures[uuid] = rootSig;
    }

    void setDescriptorHeap(RendererT uuid, ID3D12DescriptorHeap* pHeap) {
      m_pDescriptorHeaps[uuid] = pHeap;
    }

    void setPipelineState(RendererT uuid, ID3D12PipelineState* pPipeline) {
      m_pPipelineStates[uuid] = pPipeline;
    }

    U32 getFrameIndex() const { return m_frameIndex; }

    RenderPass* getBackbufferRenderPass() override { return m_pSwapchainPass; }

    RenderTargetView* getSwapchainRenderTargetView() override { 
      return &m_frameResources[m_frameIndex]._rtv;
    }

    ID3D12Resource* getFrameResourceNative() { return m_frameResources[m_frameIndex]._swapImage; }

    RendererT getSwapchainQueue() override { return kGraphicsQueueId; }

    ID3D12StateObject* getStateObject(RendererT id) {
        return m_pStateObjects[id];
    }

    D3D12_CPU_DESCRIPTOR_HANDLE getSamplerDescriptorHandle(RendererT key) {
        return m_samplers[key];
    }

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
    std::unordered_map<RendererT, std::vector<ID3D12Resource*>> m_resources;
    std::unordered_map<DescriptorHeapT, ID3D12Heap*> m_pHeaps;
    std::unordered_map<DescriptorHeapT, ID3D12DescriptorHeap*> m_pDescriptorHeaps;
    std::unordered_map<RendererT, ID3D12RootSignature*> m_pRootSignatures;
    std::unordered_map<RendererT, ID3D12CommandQueue*> m_pCommandQueues;
    std::unordered_map<RendererT, ID3D12CommandAllocator*> m_pCommandAllocators;
    std::unordered_map<RendererT, ID3D12PipelineState*> m_pPipelineStates;
    std::unordered_map<RendererT, ID3D12StateObject*> m_pStateObjects; 
    std::unordered_map<RendererT, ID3D12Fence*> m_fences;
    std::unordered_map<RendererT, HANDLE> m_fenceEvents;
    std::unordered_map<RendererT, U64> m_fenceValues;
    std::unordered_map<RendererT, std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> m_viewHandles;
    std::unordered_map<DescriptorHeapT, D3D12_CPU_DESCRIPTOR_HANDLE> m_descriptorHeapCurrentOffset; 
    std::unordered_map<RendererT, D3D12_VERTEX_BUFFER_VIEW> m_vertexBufferViews;
    std::unordered_map<RendererT, D3D12_INDEX_BUFFER_VIEW> m_indexBufferViews;
    std::unordered_map<RendererT, D3D12_CPU_DESCRIPTOR_HANDLE> m_samplers;

    MemoryAllocatorD3D12 m_memAllocator;
    ID3D12Device* m_pDevice;
    IDXGISwapChain3* m_pD3D12Swapchain;
    DXGI_SWAP_CHAIN_DESC1 m_swapchainDesc;
    RenderPassD3D12* m_pSwapchainPass;
    std::vector<FrameResource> m_frameResources; 
    ID3D12Fence* m_pPresentFence;
    HANDLE m_pPresentEvent;
    U32 m_frameIndex;

    // DirectML Operations.
    IDMLDevice* m_pdmlDevice;
    std::unordered_map<RendererT, IDMLCompiledOperator*> m_compiledOperators;
    std::unordered_map<RendererT, IDMLBindingTable*> m_bindingTables;
    std::unordered_map<RendererT, IDMLOperatorInitializer*> m_operatorInitializers;
    std::unordered_map<RendererT, IDMLCommandRecorder*> m_commandRecorders;

#if _DEBUG
    ID3D12Debug* debug0;
    ID3D12Debug1* debug1;
#endif
};


D3D12Backend* getBackendD3D12();
} // gfx