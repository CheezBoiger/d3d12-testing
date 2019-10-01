

#include "D3D12Backend.h"
#include "CommandListD3D12.h"
#include "D3D12MemAlloc.h"
#include <string>

namespace gfx
{

D3D12MA::Allocator* pAllocator = nullptr;

D3D12_RESOURCE_DIMENSION getDimension(BufferDimension dimension)
{
    switch (dimension) {
        case BUFFER_DIMENSION_BUFFER: return D3D12_RESOURCE_DIMENSION_BUFFER;
        case BUFFER_DIMENSION_2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case BUFFER_DIMENSION_3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}


D3D12_RESOURCE_STATES getNativeBindFlags(BufferBindFlags binds,
                                         D3D12_HEAP_TYPE type)
{
  D3D12_RESOURCE_STATES flags = D3D12_RESOURCE_STATE_COMMON;
  if (type == D3D12_HEAP_TYPE_READBACK)
    return D3D12_RESOURCE_STATE_COPY_DEST;
  if (type == D3D12_HEAP_TYPE_UPLOAD) 
    return D3D12_RESOURCE_STATE_GENERIC_READ;

  if (binds & BUFFER_BIND_RENDER_TARGET)
    flags |= D3D12_RESOURCE_STATE_RENDER_TARGET;
  if (binds & BUFFER_BIND_SHADER_RESOURCE)
    flags |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  if (binds & BUFFER_BIND_UNORDERED_ACCESS)
    flags |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
  if (binds & BUFFER_BIND_DEPTH_STENCIL)
    flags |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
  if (binds & BUFFER_BIND_CONSTANT_BUFFER)
    flags |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  if (binds & BUFFER_BIND_INDEX_BUFFER)
    flags |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
  if (flags & BUFFER_BIND_VERTEX_BUFFER)
    flags |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  return flags;
}


D3D12_RESOURCE_FLAGS getNativeAllowFlags(BufferBindFlags binds)
{
  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
  if (binds & BUFFER_BIND_RENDER_TARGET)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  if (binds & BUFFER_BIND_UNORDERED_ACCESS)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  if (binds & BUFFER_BIND_DEPTH_STENCIL)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  return flags;
}


void* BufferD3D12::map(U64 start, U64 sz)
{
  void* pData =  nullptr;
  D3D12_RANGE range = { };
  range.Begin = start;
  range.End = start + sz;
  ID3D12Resource* pResource = pBackend->getResource(getUUID());
  HRESULT result = pResource->Map(0, &range, &pData); 
  DX12ASSERT(result);
  return pData;
}


void BufferD3D12::unmap(U64 start, U64 sz)
{
  D3D12_RANGE range;
  range.Begin = start;
  range.End = start + sz;
  ID3D12Resource* pResource = pBackend->getResource(getUUID());
  pResource->Unmap(0, &range);
}


D3D12Backend::D3D12Backend()
{

}


IDXGIFactory4* D3D12Backend::createFactory()
{
    IDXGIFactory4* pFactory = nullptr;
    HRESULT result = CreateDXGIFactory2(0, __uuidof(IDXGIFactory4), (void**)& pFactory);
    if (FAILED(result)) {
        DEBUG("Failed to create dxgi factory.");
        return nullptr;
    }
    return pFactory;
}


void D3D12Backend::initialize(HWND handle, 
                              bool isFullScreen, 
                              const GpuConfiguration& configs)
{
    if (!handle) return;
    IDXGIFactory4* factory = createFactory();

#if _DEBUG
    D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&debug0);
    debug0->EnableDebugLayer();
    debug0->QueryInterface<ID3D12Debug1>(&debug1);
    debug1->SetEnableGPUBasedValidation(true);
#endif
    queryForDevice(factory);
    createCommandAllocators();
    createGraphicsQueue();
    createSwapChain(factory,
                    handle, 
                    configs._renderWidth,
                    configs._renderHeight,
                    configs._desiredBuffers,
                    configs._windowed);
    createHeaps();
    createDescriptorHeaps();
    querySwapChain();
    

    factory->Release();
}


void D3D12Backend::queryForDevice(IDXGIFactory4* pFactory)
{

    HRESULT result = 0;
    IDXGIAdapter1* pDesiredAdapter = nullptr;
    IDXGIAdapter1* pTempAdapter = nullptr;
    for (uint32_t i = 0; pFactory->EnumAdapters1(i, &pTempAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc = { };
        pTempAdapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }
        pDesiredAdapter = pTempAdapter;
        break;;
    }

    result = D3D12CreateDevice(pDesiredAdapter, 
                               D3D_FEATURE_LEVEL_12_1, 
                               __uuidof(ID3D12Device), 
                                (void**)& m_pDevice);
    DX12ASSERT(result);
    if (FAILED(result)) {
        DEBUG("Failed to create d3d12 device!");
        return;
    }

    D3D12MA::ALLOCATOR_DESC allocDesc = { };
    allocDesc.pDevice = m_pDevice;
    allocDesc.PreferredBlockSize = 1 * GB_1;
    allocDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
    D3D12MA::CreateAllocator(&allocDesc, &pAllocator);
}


void D3D12Backend::createSwapChain(IDXGIFactory4* pFactory,
                                   HWND handle, 
                                   U32 renderWidth, 
                                   U32 renderHeight, 
                                   U32 desiredBuffers, 
                                   B32 windowed)
{
    m_swapchainDesc = { };
    m_swapchainDesc.BufferCount = desiredBuffers;
    m_swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    m_swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_swapchainDesc.Width = renderWidth;
    m_swapchainDesc.Height = renderHeight;
    m_swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    m_swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    m_swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    m_swapchainDesc.SampleDesc.Count = 1;
    m_swapchainDesc.SampleDesc.Quality = 0;
    
    HRESULT result = pFactory->CreateSwapChainForHwnd(m_pCommandQueues[kGraphicsQueueId], 
                                                      handle, 
                                                      &m_swapchainDesc, 
                                                      nullptr, 
                                                      nullptr, 
                                                      &m_pSwapChain);
    DX12ASSERT(result);
    if (FAILED(result)) {
        DEBUG("Failed to create swapchain!");
        return;
    }

    DX12ASSERT(m_pSwapChain->QueryInterface<IDXGISwapChain3>(&m_pD3D12Swapchain));
}


void D3D12Backend::querySwapChain()
{
    m_frameResources.resize(m_swapchainDesc.BufferCount);
    DXGI_SWAP_CHAIN_DESC swapchainDesc = { };
    m_pSwapChain->GetDesc(&swapchainDesc);

    ID3D12DescriptorHeap* pRtvHeap = getDescriptorHeap(DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS);
    U32 incSz = m_pDevice->GetDescriptorHandleIncrementSize(pRtvHeap->GetDesc().Type);
    D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS];

    for (U32 i = 0; i < m_frameResources.size(); ++i) {
        FrameResource& resource = m_frameResources[i];
        ID3D12Resource* pResource = nullptr;
        HRESULT result = m_pSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&pResource);
        m_resources[m_frameResources[i]._rtv.getUUID()] = pResource;
        m_frameResources[i]._rtv._currentState = D3D12_RESOURCE_STATE_COMMON;
        D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();
        if (FAILED(result)) {
            DEBUG("Failed to query from swapchain buffer!");
            continue;
        }
        resource._swapImage = pResource;
        
        DX12ASSERT(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                          __uuidof(ID3D12CommandAllocator), 
                                          (void**)&resource._pAllocator));
        DX12ASSERT(m_pDevice->CreateCommandList(0, 
                                                 D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 resource._pAllocator, 
                                                 nullptr, 
                                                 __uuidof(ID3D12GraphicsCommandList), 
                                                 (void**)&resource._cmdList));
        resource._cmdList->Close();

        // Create fences.
        m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&resource._pSignalFence);
        resource._signalEvent = CreateEvent(0, false, false, nullptr);

        D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { };
        renderTargetViewDesc.Format = swapchainDesc.BufferDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;
        renderTargetViewDesc.Texture2D.PlaneSlice = 0;
        m_pDevice->CreateRenderTargetView(pResource, &renderTargetViewDesc, rtvHandle);
        m_viewHandles[m_frameResources[i]._rtv.getUUID()] = rtvHandle;

        rtvHandle.ptr += incSz;

        ID3D12Fence* pFence = nullptr;
        HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        DX12ASSERT(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&pFence));

        m_fences[m_frameResources[i]._pSignalFence.getUUID()] = pFence;
        m_fenceEvents[m_frameResources[i]._pSignalFence.getUUID()] = fenceEvent;
        m_fenceValues[m_frameResources[i]._pSignalFence.getUUID()] = 0;
        m_frameResources[i]._swapImage->SetName(TEXT("_swapchainBuffer"));
    }

    {
      m_pSwapchainPass = new RenderPassD3D12();
      m_pSwapchainPass->_renderTargetViews.resize(m_frameResources.size());
      for (U32 i = 0; i < m_frameResources.size(); ++i) {
        m_pSwapchainPass->_renderTargetViews[i] = &m_frameResources[i]._rtv;
      }
    }
}


void D3D12Backend::createGraphicsQueue()
{
  D3D12_COMMAND_QUEUE_DESC desc = { };
  desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  desc.Priority = 0;
  desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  desc.NodeMask = 0;
  ID3D12CommandQueue* pQueue = nullptr;
  DX12ASSERT(m_pDevice->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), (void**)&pQueue));
  m_pCommandQueues[kGraphicsQueueId] = pQueue;
}


void D3D12Backend::createBuffer(Buffer** buffer, 
                                BufferUsage usage,
                                BufferBindFlags binds, 
                                BufferDimension dimension, 
                                U32 width, 
                                U32 height,
                                U32 depth,
                                U32 structureByteStride,
                                DXGI_FORMAT format,
                                const TCHAR* debugName)
{
    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = format;

    ID3D12Resource* pResource = nullptr;
    D3D12_RESOURCE_DESC desc = { };
    desc.Alignment = 0;
    desc.Dimension = getDimension(dimension);
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = depth;
    desc.Layout = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : 
                                                                      D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Format = format;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Flags = getNativeAllowFlags(binds);

    D3D12MA::Allocation* alloc;
    D3D12MA::ALLOCATION_DESC allocDesc = { };
    allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;
    if (usage == BUFFER_USAGE_GPU_TO_CPU)
      allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
    else if (usage == BUFFER_USAGE_CPU_TO_GPU)
      allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
    else if (usage == BUFFER_USAGE_DEFAULT)
      allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_STATES state = getNativeBindFlags(binds, allocDesc.HeapType);

    D3D12_CLEAR_VALUE* pClearValue = &clearValue;
    if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
      pClearValue = NULL;
    } else {
      D3D12_RESOURCE_ALLOCATION_INFO rAllocInfo = m_pDevice->GetResourceAllocationInfo(0, 1, &desc);
      (void)rAllocInfo;
    }

    if (binds & BUFFER_BIND_RENDER_TARGET) {
      clearValue.Color[0] = 0.0f;
      clearValue.Color[1] = 0.0f;
      clearValue.Color[2] = 0.0f;
      clearValue.Color[3] = 0.0f;
    } else if (binds & BUFFER_BIND_DEPTH_STENCIL) {
      clearValue.DepthStencil.Depth = 0.0f;
      clearValue.DepthStencil.Stencil = 0;
    }
    
    HRESULT result = pAllocator->CreateResource(&allocDesc, 
                                                 &desc, 
                                                 state, 
                                                 pClearValue, 
                                                 &alloc, 
                                                 __uuidof(ID3D12Resource), 
                                                 (void**)&pResource); 
    DX12ASSERT(result);

    if (debugName) {
      pResource->SetName(debugName);
    }

    BufferD3D12* pNativeBuffer = new BufferD3D12(this,
                                                 usage,
                                                 structureByteStride);
    pNativeBuffer->pAllocation = alloc;
    pNativeBuffer->_currentResourceState = state;

    *buffer = pNativeBuffer; 

    m_resources[(*buffer)->getUUID()] = pResource;
}


void D3D12Backend::destroyBuffer(Buffer* buffer)
{
  m_memAllocator.free(m_resources[buffer->getUUID()]);
  delete buffer;
}


void D3D12Backend::createRenderPass(RenderPass** pass,
                                    U32 rtvSize,
                                    B32 hasDepthStencil)
{
  
}


void D3D12Backend::destroyRenderPass(RenderPass* pass)
{

}


void D3D12Backend::createRenderTargetView(RenderTargetView** rtv, Buffer* buffer)
{
  ViewHandleD3D12* pView = new ViewHandleD3D12();
  *rtv = pView;
  ID3D12Resource* pResource = getResource(buffer->getUUID());
  D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();
  ID3D12DescriptorHeap* rtvHeap = getDescriptorHeap(DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS);
  D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS];

  U32 incSz = m_pDevice->GetDescriptorHandleIncrementSize(rtvHeap->GetDesc().Type);

  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
  rtvDesc.Format = resourceDesc.Format;
  rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
  rtvDesc.Texture2D.MipSlice = 0;
  rtvDesc.Texture2D.PlaneSlice = 0;
  
  m_pDevice->CreateRenderTargetView(pResource, &rtvDesc, cpuHandle);
  m_viewHandles[pView->getUUID()] = cpuHandle;
  m_resources[pView->getUUID()] = pResource;

  // Set to current state, in order to transition.
  pView->_currentState = static_cast<BufferD3D12*>(buffer)->_currentResourceState;
  cpuHandle.ptr += incSz;
}


void D3D12Backend::createUnorderedAccessView(UnorderedAccessView** uav, Buffer* buffer)
{
}


void D3D12Backend::createShaderResourceView(ShaderResourceView** srv,
                                            Buffer* buffer, 
                                            U32 firstElement,
                                            U32 numElements) 
{
}


void D3D12Backend::createDepthStencilView(DepthStencilView** dsv, Buffer* buffer)
{
  ViewHandleD3D12* pView = new ViewHandleD3D12();
  *dsv = pView;
  ID3D12Resource* pResource = getResource(buffer->getUUID());
  D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();
  ID3D12DescriptorHeap* rtvHeap = getDescriptorHeap(DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS);
  D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS];

  U32 incSz = m_pDevice->GetDescriptorHandleIncrementSize(rtvHeap->GetDesc().Type);

  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
  dsvDesc.Format = resourceDesc.Format;
  dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsvDesc.Texture2D.MipSlice = 1;
  
  m_pDevice->CreateDepthStencilView(pResource, &dsvDesc, cpuHandle);
  m_viewHandles[pView->getUUID()] = cpuHandle;

  cpuHandle.ptr += incSz;
}


void D3D12Backend::present()
{
#if 1
  if (m_frameResources[m_frameIndex]._rtv._currentState != D3D12_RESOURCE_STATE_PRESENT) {
    D3D12_RESOURCE_BARRIER barrier = { };
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_frameResources[m_frameIndex]._swapImage;
    barrier.Transition.StateBefore = m_frameResources[m_frameIndex]._rtv._currentState;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_frameResources[m_frameIndex]._cmdList->Reset(m_frameResources[m_frameIndex]._pAllocator, nullptr);
    m_frameResources[m_frameIndex]._cmdList->ResourceBarrier(1, &barrier);
    HRESULT hr = m_frameResources[m_frameIndex]._cmdList->Close(); 
    DX12ASSERT(hr);
    
    ID3D12CommandList* cmd[] = { m_frameResources[m_frameIndex]._cmdList };

    m_pCommandQueues[kGraphicsQueueId]->ExecuteCommandLists(1, cmd);
    m_frameResources[m_frameIndex]._rtv._currentState = D3D12_RESOURCE_STATE_COMMON;
    
  }
#endif
  HRESULT result = m_pD3D12Swapchain->Present(1, 0); 
  DX12ASSERT(result);
  m_frameIndex = m_pD3D12Swapchain->GetCurrentBackBufferIndex();
}


void D3D12Backend::createHeaps()
{
  for (U32 i = 0; i < DESCRIPTOR_HEAP_END; ++i) {

    D3D12_HEAP_DESC heapDesc = { };
    heapDesc.Properties.VisibleNodeMask = 0;
    heapDesc.Alignment = 0;
    heapDesc.Properties.CreationNodeMask = 0;
    heapDesc.SizeInBytes = 512 * MB_1; 
    switch (i) {
      case DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS:
      case DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS:
        // My device seems to break when CPUPageProperty and/or MemoryPool Preference are defined,
        // which doesn't seem to make sense why they exist when heap type already defines the mem type.
        heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES; 
        heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        break;
      default:
        heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
        heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    }

    ID3D12Heap* pHeap = nullptr;
    HRESULT result = m_pDevice->CreateHeap(&heapDesc, __uuidof(ID3D12Heap), (void**)&pHeap); 
    DX12ASSERT(result);

    m_pHeaps[i] = pHeap;
  }
}


void D3D12Backend::createDescriptorHeaps()
{
  for (U32 i = DESCRIPTOR_HEAP_START; i < DESCRIPTOR_HEAP_END; ++i) {
    D3D12_DESCRIPTOR_HEAP_DESC desc = { };
    desc.NodeMask = 0;
    desc.NumDescriptors = 2048;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    switch (i) {
      case DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS:
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        break;
      case DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS:
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        break;
      case DESCRIPTOR_HEAP_SAMPLER:
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        break;
      default:
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        break;
    }

    ID3D12DescriptorHeap* pDescHeap = nullptr;
    DX12ASSERT(m_pDevice->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap), (void**)&pDescHeap));
    m_pDescriptorHeaps[i] = pDescHeap;
    m_descriptorHeapCurrentOffset[i] = pDescHeap->GetCPUDescriptorHandleForHeapStart();
  }
}


void D3D12Backend::createCommandList(CommandList** pList) 
{
  std::vector<ID3D12CommandAllocator*> allocs(m_frameResources.size());
  for (U32 i = 0; i < m_frameResources.size(); ++i)
    allocs[i] = m_frameResources[i]._pAllocator;

  CommandList* pNativeList = nullptr;
  pNativeList = new GraphicsCommandListD3D12(this,
                                            D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                            allocs.data(), 
                                            static_cast<U32>(allocs.size()));

  *pList = pNativeList;
  
}


void D3D12Backend::destroyCommandList(CommandList* pList)
{
  pList->destroy();
}


void D3D12Backend::submit(RendererT queue, CommandList** cmdLists, U32 numCmdLists)
{
  ID3D12CommandQueue* pQueue = m_pCommandQueues[queue];
  static ID3D12CommandList* pNativeLists[64];

  for (U32 i = 0; i < numCmdLists; ++i) {
    GraphicsCommandListD3D12* pCmdList = static_cast<GraphicsCommandListD3D12*>(cmdLists[i]); 
    pNativeLists[i] = pCmdList->getNativeList(m_frameIndex);
    if (pCmdList->isRecording()) {
      ASSERT(false && "Cmd list submitted for execution, when it is still in record mode!");
    }
  }

  pQueue->ExecuteCommandLists(numCmdLists, pNativeLists);
}


void D3D12Backend::signalFence(RendererT queue, Fence* fence)
{
  RendererT f = fence->getUUID();
  ID3D12CommandQueue* pQueue = m_pCommandQueues[queue];
  ID3D12Fence* pFence = m_fences[f];
  
  DX12ASSERT(pQueue->Signal(pFence, m_fenceValues[f]));
}


void D3D12Backend::waitFence(Fence* fence)
{
  RendererT f = fence->getUUID();
  ID3D12Fence* pFence = m_fences[f];
  pFence->SetEventOnCompletion(m_fenceValues[f], m_fenceEvents[f]);
  WaitForSingleObject(m_fenceEvents[f], INFINITE);
  m_fenceValues[f]++;
  // No resetting the allocator can lead to mem leaks.
  DX12ASSERT(m_frameResources[m_frameIndex]._pAllocator->Reset());
}
} // gfx