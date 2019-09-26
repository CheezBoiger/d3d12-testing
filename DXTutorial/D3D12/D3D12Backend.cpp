

#include "D3D12Backend.h"

namespace gfx
{


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
                              const GraphicsConfiguration& configs)
{
    if (!handle) return;
    IDXGIFactory4* factory = createFactory();

    queryForDevice(factory);
    createCommandAllocators();
    createSwapChain(factory,
                    handle, 
                    configs._renderWidth,
                    configs._renderHeight,
                    configs._desiredBuffers,
                    configs._windowed);
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
        return;
    }

    result = D3D12CreateDevice(pDesiredAdapter, 
                               D3D_FEATURE_LEVEL_11_1, 
                               __uuidof(ID3D12Device), 
                               (void**)& m_pDevice);

    if (FAILED(result)) {
        DEBUG("Failed to create d3d12 device!");
        return;
    }
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
    
    HRESULT result = pFactory->CreateSwapChainForHwnd(m_pDevice, 
                                                      handle, 
                                                      &m_swapchainDesc, 
                                                      nullptr, 
                                                      nullptr, 
                                                      &m_pSwapChain);
    
    if (FAILED(result)) {
        DEBUG("Failed to create swapchain!");
        return;
    }
}


void D3D12Backend::querySwapChain()
{
    m_frameResources.resize(m_swapchainDesc.BufferCount);
    for (U32 i = 0; i < m_frameResources.size(); ++i) {
        FrameResource& resource = m_frameResources[i];
        ID3D12Resource* pResource = nullptr;
        HRESULT result = m_pSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&pResource);
        if (FAILED(result)) {
            DEBUG("Failed to query from swapchain buffer!");
            continue;
        }
        resource._swapImage = pResource;
        
        m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                          __uuidof(ID3D12CommandAllocator), 
                                          (void**)&resource._pAllocator);
        m_pDevice->CreateCommandList(0, 
                                     D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                     resource._pAllocator, 
                                     nullptr, 
                                     __uuidof(ID3D12CommandList), 
                                     (void**)&resource._pCmdList);

        // Create fences.
        m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&resource._pSignalFence);
        resource._signalEvent = CreateEvent(0, false, false, nullptr);
    }
}
}