

#include "D3D12Backend.h"
#include "CommandListD3D12.h"
#include "D3D12MemAlloc.h"
#include "RootSignatureD3D12.h"
#include "PipelineStatesD3D12.h"
#include "DescriptorTableD3D12.h"
#include <string>

namespace gfx
{

D3D12Backend* getBackendD3D12()
{
  static D3D12Backend backend;
  return &backend;
}

D3D12MA::Allocator* pAllocator = nullptr;

D3D12_RESOURCE_DIMENSION getDimension(ResourceDimension dimension)
{
    switch (dimension) {
        case RESOURCE_DIMENSION_2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case RESOURCE_DIMENSION_3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}


D3D12_RESOURCE_STATES getNativeBindFlags(ResourceBindFlags binds,
                                         D3D12_HEAP_TYPE type)
{
  D3D12_RESOURCE_STATES flags = D3D12_RESOURCE_STATE_COMMON;
  if (type == D3D12_HEAP_TYPE_READBACK)
    return D3D12_RESOURCE_STATE_COPY_DEST;
  if (type == D3D12_HEAP_TYPE_UPLOAD) 
    return D3D12_RESOURCE_STATE_GENERIC_READ;

  if (binds & RESOURCE_BIND_RENDER_TARGET)
    flags |= D3D12_RESOURCE_STATE_RENDER_TARGET;
  if (binds & RESOURCE_BIND_SHADER_RESOURCE)
    flags |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  if (binds & RESOURCE_BIND_UNORDERED_ACCESS)
    flags |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
  if (binds & RESOURCE_BIND_DEPTH_STENCIL)
    flags |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
  if (binds & RESOURCE_BIND_CONSTANT_BUFFER)
    flags |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  if (binds & RESOURCE_BIND_INDEX_BUFFER)
    flags |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
  if (flags & RESOURCE_BIND_VERTEX_BUFFER)
    flags |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  return flags;
}


D3D12_RESOURCE_FLAGS getNativeAllowFlags(ResourceBindFlags binds)
{
  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
  if (binds & RESOURCE_BIND_RENDER_TARGET)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  if (binds & RESOURCE_BIND_UNORDERED_ACCESS)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  if (binds & RESOURCE_BIND_DEPTH_STENCIL)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  return flags;
}


D3D12_PRIMITIVE_TOPOLOGY_TYPE getNativeTopologyType(PrimitiveTopology topology)
{
  switch (topology) {
    case PRIMITIVE_TOPOLOGY_POINTS: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case PRIMITIVE_TOPOLOGY_LINES: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case PRIMITIVE_TOPOLOGY_TRIANGLES: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case PRIMITIVE_TOPOLOGY_PATCHES: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    default: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
  }
}


D3D12_CULL_MODE getCullMode(CullMode cullMode)
{
  switch (cullMode) {
    case CULL_MODE_BACK: return D3D12_CULL_MODE_BACK;
    case CULL_MODE_NONE: return D3D12_CULL_MODE_NONE;
    case CULL_MODE_FRONT: 
    default: return D3D12_CULL_MODE_FRONT;
  }
}


D3D12_FILL_MODE getFillMode(FillMode fillMode)
{
  switch (fillMode) {
    case FILL_MODE_WIREFRAME: return D3D12_FILL_MODE_WIREFRAME;
    case FILL_MODE_SOLID:
    default: return D3D12_FILL_MODE_SOLID;
  }
}


D3D12_INDEX_BUFFER_STRIP_CUT_VALUE getIBCutValue(IBCutValue cutValue)
{
  switch (cutValue) {
    case IB_CUT_VALUE_CUT_0xFFFF: return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
    case IB_CUT_VALUE_CUT_0xFFFFFFFF: return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
    case IB_CUT_VALUE_DISABLED: 
    default: return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
  }
}


D3D12_STENCIL_OP getStencilOp(StencilOp op)
{
  switch (op) {
    case STENCIL_OP_DECR: return D3D12_STENCIL_OP_DECR;
    case STENCIL_OP_DECR_SAT: return D3D12_STENCIL_OP_DECR_SAT;
    case STENCIL_OP_INCR: return D3D12_STENCIL_OP_INCR;
    case STENCIL_OP_INCR_SAT: return D3D12_STENCIL_OP_INCR_SAT;
    case STENCIL_OP_INVERT: return D3D12_STENCIL_OP_INVERT;
    case STENCIL_OP_KEEP: return D3D12_STENCIL_OP_KEEP;
    case STENCIL_OP_REPLACE: return D3D12_STENCIL_OP_REPLACE;
    case STENCIL_OP_ZERO: 
    default: return D3D12_STENCIL_OP_ZERO; 
  }
}


D3D12_COMPARISON_FUNC getComparisonFunc(ComparisonFunc func)
{
  switch (func) {
    case COMPARISON_FUNC_EQUAL: return D3D12_COMPARISON_FUNC_EQUAL;
    case COMPARISON_FUNC_GREATER: return D3D12_COMPARISON_FUNC_GREATER;
    case COMPARISON_FUNC_GREATER_EQUAL: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case COMPARISON_FUNC_LESS: return D3D12_COMPARISON_FUNC_LESS;
    case COMPARISON_FUNC_LESS_EQUAL: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case COMPARISON_FUNC_NOT_EQUAL: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case COMPARSON_FUNC_ALWAYS: return D3D12_COMPARISON_FUNC_ALWAYS; 
    case COMPARISON_FUNC_NEVER:
    default: return D3D12_COMPARISON_FUNC_NEVER;
  }
}


D3D12_DEPTH_WRITE_MASK getDepthWriteMask(DepthWriteMask mask)
{
  switch (mask) {
    case DEPTH_WRITE_MASK_ALL: return D3D12_DEPTH_WRITE_MASK_ALL;
    case DEPTH_WRITE_MASK_ZERO:
    default: return D3D12_DEPTH_WRITE_MASK_ZERO;
  }
}


void processRasterizationState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, 
                               const RasterizationState& rasterState)
{
  desc.RasterizerState.AntialiasedLineEnable = rasterState._antialiasedLinesEnable;
  desc.RasterizerState.ConservativeRaster = rasterState._conservativeRasterizationEnable 
                                          ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON 
                                          : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF; 
  desc.RasterizerState.CullMode = getCullMode(rasterState._cullMode);
  desc.RasterizerState.FillMode = getFillMode(rasterState._fillMode);
  desc.RasterizerState.DepthBias = rasterState._depthBias;
  desc.RasterizerState.MultisampleEnable = rasterState._multisampleEnable;
  desc.RasterizerState.DepthClipEnable = rasterState._depthClipEnable;
  desc.RasterizerState.DepthBiasClamp = rasterState._depthBiasClamp;
  desc.RasterizerState.ForcedSampleCount = rasterState._forcedSampleCount;
  desc.RasterizerState.FrontCounterClockwise = rasterState._frontCounterClockwise;
  desc.RasterizerState.SlopeScaledDepthBias = rasterState._slopedScaledDepthBias;
}


void processDepthStencilState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
                              const DepthStencilState& dsState)
{
  desc.DepthStencilState.BackFace.StencilDepthFailOp = getStencilOp(dsState._backFace._stencilDepthFailOp);
  desc.DepthStencilState.BackFace.StencilFailOp = getStencilOp(dsState._backFace._stencilFailOp);
  desc.DepthStencilState.BackFace.StencilPassOp = getStencilOp(dsState._backFace._stencilPassOp);
  desc.DepthStencilState.BackFace.StencilFunc = getComparisonFunc(dsState._backFace._stencilFunc);

  desc.DepthStencilState.FrontFace.StencilDepthFailOp = getStencilOp(dsState._frontFace._stencilDepthFailOp);
  desc.DepthStencilState.FrontFace.StencilFailOp = getStencilOp(dsState._frontFace._stencilFailOp);
  desc.DepthStencilState.FrontFace.StencilPassOp = getStencilOp(dsState._frontFace._stencilPassOp);
  desc.DepthStencilState.FrontFace.StencilFunc = getComparisonFunc(dsState._frontFace._stencilFunc);

  desc.DepthStencilState.DepthEnable = dsState._depthEnable;
  desc.DepthStencilState.DepthFunc = getComparisonFunc(dsState._depthFunc);
  desc.DepthStencilState.DepthWriteMask = getDepthWriteMask(dsState._depthWriteMask);
  desc.DepthStencilState.StencilEnable = dsState._stencilEnable;
  desc.DepthStencilState.StencilReadMask = dsState._stencilReadMask;
  desc.DepthStencilState.StencilWriteMask = dsState._stencilWriteMask;
}


D3D12_BLEND getBlend(Blend b)
{
  switch (b) {
    case BLEND_BLEND_FACTOR: return D3D12_BLEND_BLEND_FACTOR;
    case BLEND_DEST_ALPHA: return D3D12_BLEND_DEST_ALPHA;
    case BLEND_DEST_COLOR: return D3D12_BLEND_DEST_COLOR;
    case BLEND_INV_BLEND_FACTOR: return D3D12_BLEND_INV_BLEND_FACTOR;
    case BLEND_INV_DEST_ALPHA: return D3D12_BLEND_INV_DEST_ALPHA;
    case BLEND_INV_DEST_COLOR: return D3D12_BLEND_INV_DEST_COLOR;
    case BLEND_INV_SRC1_ALPHA: return D3D12_BLEND_INV_SRC1_ALPHA;
    case BLEND_INV_SRC1_COLOR: return D3D12_BLEND_INV_SRC1_COLOR;
    case BLEND_INV_SRC_COLOR: return D3D12_BLEND_INV_SRC_COLOR;
    case BLEND_ONE: return D3D12_BLEND_ONE;
    case BLEND_ZERO: return D3D12_BLEND_ZERO;
    default: return D3D12_BLEND_ZERO;
  }
}


D3D12_BLEND_OP getBlendOp(BlendOp op)
{
  switch (op) {
    case BLEND_OP_SUBTRACT: return D3D12_BLEND_OP_SUBTRACT;
    case BLEND_OP_REV_SUBTRACT: return D3D12_BLEND_OP_REV_SUBTRACT;
    case BLEND_OP_MIN: return D3D12_BLEND_OP_MIN;
    case BLEND_OP_MAX: return D3D12_BLEND_OP_MAX;
    case BLEND_OP_ADD: 
    default: return D3D12_BLEND_OP_ADD;
  }
}


D3D12_LOGIC_OP getLogicOp(LogicOp op)
{
  switch (op) {
    case LOGIC_OP_SET: return D3D12_LOGIC_OP_SET;
    case LOGIC_OP_COPY: return D3D12_LOGIC_OP_COPY;
    case LOGIC_OP_COPY_INVERTED: return D3D12_LOGIC_OP_COPY_INVERTED;
    case LOGIC_OP_NOOP: return D3D12_LOGIC_OP_NOOP;
    case LOGIC_OP_INVERT: return D3D12_LOGIC_OP_INVERT;
    case LOGIC_OP_AND: return D3D12_LOGIC_OP_AND;
    case LOGIC_OP_NAND: return D3D12_LOGIC_OP_NAND;
    case LOGIC_OP_OR: return D3D12_LOGIC_OP_OR;
    case LOGIC_OP_NOR: return D3D12_LOGIC_OP_NOR;
    case LOGIC_OP_XOR: return D3D12_LOGIC_OP_XOR;
    case LOGIC_OP_EQUIV: return D3D12_LOGIC_OP_EQUIV;
    case LOGIC_OP_AND_REVERSE: return D3D12_LOGIC_OP_AND_REVERSE;
    case LOGIC_OP_AND_INVERTED: return D3D12_LOGIC_OP_AND_INVERTED;
    case LOGIC_OP_OR_REVERSE: return D3D12_LOGIC_OP_OR_REVERSE;
    case LOGIC_OP_OR_INVERTED: return D3D12_LOGIC_OP_OR_INVERTED;
    case LOGIC_OP_CLEAR:
    default: return D3D12_LOGIC_OP_CLEAR;
  }
}


void processBlendState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
                       const BlendState& blendState)
{
  desc.BlendState.AlphaToCoverageEnable = blendState._alhpaToCoverageEnable;
  desc.BlendState.IndependentBlendEnable = blendState._independentBlendEnable;
  
  for (U32 i = 0; i < 8; ++i) {
      desc.BlendState.RenderTarget[i].BlendEnable = blendState._renderTargets[i]._blendEnable;
      desc.BlendState.RenderTarget[i].BlendOp = getBlendOp(blendState._renderTargets[i]._blendOp);
      desc.BlendState.RenderTarget[i].BlendOpAlpha = getBlendOp(blendState._renderTargets[i]._blendOpAlpha);
      desc.BlendState.RenderTarget[i].DestBlend = getBlend(blendState._renderTargets[i]._dstBlend);
      desc.BlendState.RenderTarget[i].DestBlendAlpha = getBlend(blendState._renderTargets[i]._dstBlendAlpha);
      desc.BlendState.RenderTarget[i].LogicOp = getLogicOp(blendState._renderTargets[i]._logicOp);
      desc.BlendState.RenderTarget[i].LogicOpEnable = blendState._renderTargets[i]._logicOpEnable;
      desc.BlendState.RenderTarget[i].RenderTargetWriteMask = blendState._renderTargets[i]._renderTargetWriteMask;
      desc.BlendState.RenderTarget[i].SrcBlend = getBlend(blendState._renderTargets[i]._srcBlend);
      desc.BlendState.RenderTarget[i].SrcBlendAlpha = getBlend(blendState._renderTargets[i]._srcBlendAlpha);
  }
}
    

void processInputLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
                        const InputLayout& layout)
{
  desc.InputLayout.pInputElementDescs;
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

    // Check if supports raytracing.
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureOptions;
    DX12ASSERT(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, 
                                              &featureOptions, 
                                              sizeof(featureOptions)));
    m_rayTracingHardwareCompatible = (featureOptions.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);

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
        m_resources[FrameResource::kFrameResourceId].push_back(pResource);
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

        D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { };
        renderTargetViewDesc.Format = swapchainDesc.BufferDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;
        renderTargetViewDesc.Texture2D.PlaneSlice = 0;
        m_pDevice->CreateRenderTargetView(pResource, &renderTargetViewDesc, rtvHandle);
        m_viewHandles[m_frameResources[i]._rtv.getUUID()].push_back(rtvHandle);

        rtvHandle.ptr += incSz;
        m_frameResources[i]._swapImage->SetName(TEXT("_swapchainBuffer"));
        m_frameResources[i]._fenceValue = 0;
        m_frameResources[i]._rtv._buffer = FrameResource::kFrameResourceId;
    }

    {
      m_pSwapchainPass = new RenderPassD3D12();
      m_pSwapchainPass->_renderTargetViews.resize(m_frameResources.size());
      for (U32 i = 0; i < m_frameResources.size(); ++i) {
        m_pSwapchainPass->_renderTargetViews[i] = &m_frameResources[i]._rtv;
      }
    }

    DX12ASSERT(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pPresentFence));
    m_pPresentEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
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


void D3D12Backend::createBuffer(Resource** buffer, 
                                ResourceUsage usage,
                                ResourceBindFlags binds, 
                                U32 widthBytes, 
                                U32 structureByteStride,
                                const TCHAR* debugName)
{
    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = DXGI_FORMAT_UNKNOWN;

    ID3D12Resource* pResource = nullptr;
    D3D12_RESOURCE_DESC desc = { };
    desc.Alignment = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = widthBytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Flags = getNativeAllowFlags(binds);
  
    D3D12MA::Allocation* alloc;
    D3D12MA::ALLOCATION_DESC allocDesc = { };
    allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;
    if (usage == RESOURCE_USAGE_GPU_TO_CPU)
      allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
    else if (usage == RESOURCE_USAGE_CPU_TO_GPU)
      allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
    else if (usage == RESOURCE_USAGE_DEFAULT)
      allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_STATES state = getNativeBindFlags(binds, allocDesc.HeapType);

    D3D12_CLEAR_VALUE* pClearValue = &clearValue;
    if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
      pClearValue = NULL;
    } else {
      D3D12_RESOURCE_ALLOCATION_INFO rAllocInfo = m_pDevice->GetResourceAllocationInfo(0, 1, &desc);
      (void)rAllocInfo;
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
                                                 RESOURCE_DIMENSION_BUFFER,
                                                 usage,
                                                 binds,
                                                 structureByteStride);
    pNativeBuffer->pAllocation = alloc;
    pNativeBuffer->_currentResourceState = state;

    *buffer = pNativeBuffer; 

    m_resources[(*buffer)->getUUID()].push_back(pResource);
}


void D3D12Backend::createTexture(Resource** texture,
                                 ResourceDimension dimension,
                                 ResourceUsage usage,
                                 ResourceBindFlags binds,
                                 DXGI_FORMAT format,
                                 U32 width,
                                 U32 height,
                                 U32 depth,
                                 U32 structureByteStride,
                                 const TCHAR* debugName)
{
  D3D12_CLEAR_VALUE clearValue;
  clearValue.Format = format;

  ID3D12Resource* pResource = nullptr;
  D3D12_RESOURCE_DESC desc = {};
  desc.Alignment = 0;
  desc.Dimension = getDimension(dimension);
  desc.Width = width;
  desc.Height = height;
  desc.DepthOrArraySize = depth;
  desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  desc.Format = format;
  desc.MipLevels = 1;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Flags = getNativeAllowFlags(binds);

  D3D12MA::Allocation* alloc;
  D3D12MA::ALLOCATION_DESC allocDesc = {};
  allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;
  if (usage == RESOURCE_USAGE_GPU_TO_CPU)
    allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
  else if (usage == RESOURCE_USAGE_CPU_TO_GPU)
    allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
  else if (usage == RESOURCE_USAGE_DEFAULT)
    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

  D3D12_RESOURCE_STATES state = getNativeBindFlags(binds, allocDesc.HeapType);

  D3D12_CLEAR_VALUE* pClearValue = &clearValue;
  if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
    pClearValue = NULL;
  } else {
    D3D12_RESOURCE_ALLOCATION_INFO rAllocInfo =
        m_pDevice->GetResourceAllocationInfo(0, 1, &desc);
    (void)rAllocInfo;
  }

  if (binds & RESOURCE_BIND_RENDER_TARGET) {
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 0.0f;
  } else if (binds & RESOURCE_BIND_DEPTH_STENCIL) {
    clearValue.DepthStencil.Depth = 0.0f;
    clearValue.DepthStencil.Stencil = 0;
  }

  HRESULT result =
      pAllocator->CreateResource(&allocDesc, 
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

  BufferD3D12* pNativeBuffer = new BufferD3D12(this, dimension, usage, binds, structureByteStride);
  pNativeBuffer->pAllocation = alloc;
  pNativeBuffer->_currentResourceState = state;

  *texture = pNativeBuffer;

  m_resources[(*texture)->getUUID()].push_back(pResource);
}


void D3D12Backend::destroyResource(Resource* buffer)
{
  for (size_t i = 0; i < m_resources[buffer->getUUID()].size(); ++i) {
    m_resources[buffer->getUUID()][i]->Release();
    m_resources[buffer->getUUID()][i] = nullptr;
  }
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


void D3D12Backend::createRenderTargetView(RenderTargetView** rtv, Resource* buffer)
{
  ViewHandleD3D12* pView = new ViewHandleD3D12();
  ID3D12DescriptorHeap* rtvHeap = getDescriptorHeap(DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS);
  D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS];

  U32 incSz = m_pDevice->GetDescriptorHandleIncrementSize(rtvHeap->GetDesc().Type);

  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };

  rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
  rtvDesc.Texture2D.MipSlice = 0;
  rtvDesc.Texture2D.PlaneSlice = 0;
  
  m_viewHandles[pView->getUUID()].resize(m_resources[buffer->getUUID()].size());

  for (size_t i = 0; i < m_resources[buffer->getUUID()].size(); ++i) {
    ID3D12Resource* pResource = getResource(buffer->getUUID(), i);
    D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();
    rtvDesc.Format = resourceDesc.Format;
    m_pDevice->CreateRenderTargetView(pResource, &rtvDesc, cpuHandle);
    m_viewHandles[pView->getUUID()][i] = cpuHandle;
    cpuHandle.ptr += incSz;
  }

  // Set to current state, in order to transition.
  pView->_currentState = static_cast<BufferD3D12*>(buffer)->_currentResourceState;
  pView->_buffer = buffer->getUUID();

  *rtv = pView;
}


void D3D12Backend::createUnorderedAccessView(UnorderedAccessView** uav, Resource* buffer)
{

}


void D3D12Backend::createShaderResourceView(ShaderResourceView** srv,
                                            Resource* buffer, 
                                            U32 firstElement,
                                            U32 numElements) 
{
}


void D3D12Backend::createDepthStencilView(DepthStencilView** dsv, Resource* buffer)
{
  ViewHandleD3D12* pView = new ViewHandleD3D12();
  *dsv = pView;
  pView->_buffer = buffer->getUUID();

  ID3D12DescriptorHeap* rtvHeap = getDescriptorHeap(DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS);
  D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS];

  U32 incSz = m_pDevice->GetDescriptorHandleIncrementSize(rtvHeap->GetDesc().Type);

  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
  dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsvDesc.Texture2D.MipSlice = 0;
  
  m_viewHandles[pView->getUUID()].resize(m_resources[buffer->getUUID()].size());
  for (size_t i = 0; i < m_resources[buffer->getUUID()].size(); ++i) {
    ID3D12Resource* pResource = getResource(buffer->getUUID(), i);
    D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();
    dsvDesc.Format = resourceDesc.Format;
    m_pDevice->CreateDepthStencilView(pResource, &dsvDesc, cpuHandle);
    m_viewHandles[pView->getUUID()][i] = cpuHandle;
    cpuHandle.ptr += incSz;
  }
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

  U32 currFenceValue = m_frameResources[m_frameIndex]._fenceValue;
  m_pCommandQueues[kGraphicsQueueId]->Signal(m_pPresentFence, currFenceValue);
  m_frameIndex = m_pD3D12Swapchain->GetCurrentBackBufferIndex();
  if (m_pPresentFence->GetCompletedValue() < m_frameResources[m_frameIndex]._fenceValue) {
    m_pPresentFence->SetEventOnCompletion(m_frameResources[m_frameIndex]._fenceValue, m_pPresentEvent);
    WaitForSingleObjectEx(m_pPresentEvent, INFINITE, FALSE);
  }

  m_frameResources[m_frameIndex]._fenceValue = currFenceValue + 1;

  DX12ASSERT(m_frameResources[m_frameIndex]._pAllocator->Reset());
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
    DX12ASSERT(m_pDevice->CreateDescriptorHeap(&desc, 
                                               __uuidof(ID3D12DescriptorHeap), 
                                               (void**)&pDescHeap));
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
  pNativeList = new GraphicsCommandListD3D12(D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                            allocs.data(), 
                                            static_cast<U32>(allocs.size()));

  *pList = pNativeList;
  
}


void D3D12Backend::destroyCommandList(CommandList* pList)
{
  pList->destroy();
  delete pList;
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
}



void D3D12Backend::createDescriptorTable(DescriptorTable** table)
{
  DescriptorTableD3D12* pHeap = new DescriptorTableD3D12();
  *table = pHeap;
}


void D3D12Backend::createRootSignature(RootSignature** ppRootSig)
{
  RootSignatureD3D12* pRootSignature = new RootSignatureD3D12();
  *ppRootSig = pRootSignature;
}


void D3D12Backend::createVertexBufferView(VertexBufferView** ppView,
                                          Resource* pBuffer,
                                          U32 vertexStride,
                                          U32 bufferSzBytes)
{
  VertexBufferViewD3D12* pNativeView = new VertexBufferViewD3D12();
  *ppView = pNativeView;
  pNativeView->_buffer = pBuffer->getUUID();
  pNativeView->_szInBytes = bufferSzBytes;
  pNativeView->_vertexStrideBytes = vertexStride;
}


void D3D12Backend::createIndexBufferView(IndexBufferView** ppView,
                                         Resource* pBuffer,
                                         DXGI_FORMAT format,
                                         U32 szBytes)
{
  IndexBufferViewD3D12* pNativeView = new IndexBufferViewD3D12();
  *ppView = pNativeView;
  pNativeView->_buffer = pBuffer->getUUID();
  pNativeView->_format = format;
  pNativeView->_szBytes = szBytes;
}


void D3D12Backend::createFence(Fence** ppFence)
{
  Fence* pFence = new Fence();
  ID3D12Fence* pNativeFence = nullptr;
  DX12ASSERT(m_pDevice->CreateFence(0, 
                                    D3D12_FENCE_FLAG_NONE,
                                    __uuidof(ID3D12Fence), 
                                    (void**)&pNativeFence));
  *ppFence = pFence;
  m_fences[pFence->getUUID()] = pNativeFence;
  m_fenceEvents[pFence->getUUID()] = CreateEvent(NULL, FALSE, FALSE, NULL);
  m_fenceValues[pFence->getUUID()] = 1;
}


void D3D12Backend::destroyFence(Fence* pFence)
{
  m_fences[pFence->getUUID()]->Release();
  m_fences[pFence->getUUID()] = nullptr;
  delete pFence;
}


void D3D12Backend::createGraphicsPipelineState(GraphicsPipeline** ppPipeline,
                                               const GraphicsPipelineInfo* pInfo)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = { };
    desc.VS.BytecodeLength = pInfo->_vertexShader._szBytes;
    desc.VS.pShaderBytecode = pInfo->_vertexShader._pByteCode;

    desc.PS.BytecodeLength = pInfo->_pixelShader._szBytes;
    desc.PS.pShaderBytecode = pInfo->_pixelShader._pByteCode;

    desc.DS.BytecodeLength = pInfo->_domainShader._szBytes;
    desc.DS.pShaderBytecode = pInfo->_domainShader._pByteCode;

    desc.HS.BytecodeLength = pInfo->_hullShader._szBytes;
    desc.HS.pShaderBytecode = pInfo->_hullShader._pByteCode;

    desc.GS.BytecodeLength = pInfo->_geometryShader._szBytes;
    desc.GS.pShaderBytecode = pInfo->_geometryShader._pByteCode;

    desc.PrimitiveTopologyType = getNativeTopologyType(pInfo->_topology);
    desc.DSVFormat = pInfo->_dsvFormat;
    desc.NumRenderTargets = pInfo->_numRenderTargets;
    desc.NodeMask = 0;
    desc.pRootSignature = getBackendD3D12()->getRootSignature(pInfo->_pRootSignature->getUUID());
    
    processRasterizationState(desc, pInfo->_rasterizationState);
    processDepthStencilState(desc, pInfo->_depthStencilState);
    processBlendState(desc, pInfo->_blendState);
    processInputLayout(desc, pInfo->_inputLayout);

    desc.IBStripCutValue = getIBCutValue(pInfo->_ibCutValue);
    desc.StreamOutput;
    desc.SampleMask = pInfo->_sampleMask;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    for (U32 i = 0; i < 8; ++i) {
      desc.RTVFormats[i] = pInfo->_rtvFormats[i];
    }


    ID3D12PipelineState* pPipelineState = nullptr;
  
    DX12ASSERT(m_pDevice->CreateGraphicsPipelineState(&desc, 
                                                      __uuidof(ID3D12PipelineState), 
                                                      (void**)&pPipelineState));
    *ppPipeline = new GraphicsPipelineStateD3D12();
    m_pPipelineStates[(*ppPipeline)->getUUID()] = pPipelineState;
}


void D3D12Backend::createComputePipelineState(ComputePipeline** ppPipeline,
                                              const ComputePipelineInfo* pInfo)
{
  *ppPipeline = new ComputePipelineStateD3D12();

  D3D12_COMPUTE_PIPELINE_STATE_DESC compDesc = { };

  ID3D12PipelineState* pPipelineState = nullptr;

  DX12ASSERT(m_pDevice->CreateComputePipelineState(&compDesc, 
                                                   __uuidof(ID3D12PipelineState), 
                                                   (void**)&pPipelineState));

  m_pPipelineStates[(*ppPipeline)->getUUID()] = pPipelineState;
}
} // gfx