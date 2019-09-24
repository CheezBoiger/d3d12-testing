
#pragma once

#include "../Renderer.h"

#include <d3d12.h>
#include <d3d12shader.h>


#include <vector>
#include <unordered_map>

namespace gfx 
{

class GraphicsCommandListD3D12 : CommandList
{
public:
    GraphicsCommandListD3D12(ID3D12CommandAllocator* pAlloc);
    
    void reset() override; 
    void drawIndexedInstanced() override;
    void drawInstanced() override;
    void setPipelineStateObject() override;
    void setRenderTargets() override;
    void dispatch() override;
    void setVertexBuffers() override;
    void setIndexBuffer() override; 
    void close() override;
    void setViewports() override;
    void setScissors() override;

private:
    ID3D12GraphicsCommandList* m_pCmdList;
};

// Frame Resources.
struct FrameResource
{
    ID3D12CommandAllocator* _pAllocator;
    ID3D12CommandList* _pCmdList;
    ID3D12Resource* _swapImage;
    ID3D12Fence* _pSignalFence;
    HANDLE _signalEvent;
};

class D3D12Backend : public BackendRenderer
{
public:

    D3D12Backend();

    void initialize(HWND handle, const GraphicsConfiguration& configs) override;
    void cleanUp() override;

    void present() override;
    void submit(RendererT queue, RendererT* cmdList, U32 numCmdLists) override;
    void signalFence(RendererT queue, HANDLE fence) override;

private:

    void queryForDevice(IDXGIFactory4* pFactory);
    void createSwapChain(IDXGIFactory4* pFactory,
                         HWND handle, 
                         U32 renderWidth, 
                         U32 renderHeight, 
                         U32 desiredBuffers, 
                         B32 windowed);
    void querySwapChain();
    IDXGIFactory4* createFactory();
    void createCommandAllocators();

    void onCreateCommandList(CommandList** pList, RendererT assignedUUID) override;
    void onCreateBuffer(Buffer** pBuf, RendererT assignedUUID) override;
    

    std::unordered_map<RendererT, CommandList*> m_cmdLists;
    std::unordered_map<RendererT, ID3D12Resource*> m_resources;
    std::unordered_map<RendererT, ID3D12Heap*> m_pHeaps;
    std::unordered_map<RendererT, ID3D12DescriptorHeap*> m_pDescriptorHeaps;
    std::unordered_map<RendererT, ID3D12RootSignature*> m_pRootSignatures;
    std::unordered_map<RendererT, ID3D12CommandQueue*> m_pCommandQueues;
    std::unordered_map<RendererT, ID3D12CommandAllocator*> m_pCommandAllocators;
    std::unordered_map<RendererT, ID3D12PipelineState*> m_pPipelineStates;

    ID3D12Device* m_pDevice;
    
    IDXGISwapChain1* m_pSwapChain;
    DXGI_SWAP_CHAIN_DESC1 m_swapchainDesc;
    std::vector<FrameResource> m_frameResources; 
    U32 m_frameIndex;
};
} // gfx