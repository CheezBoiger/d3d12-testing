

#pragma once

#include "../Renderer.h"

#include "CommonsD3D11.h"
#include "CommandListD3D11.h"

#include <vector>
#include <unordered_map>

namespace gfx {

class D3D11Backend : public BackendRenderer
{
public:

    void initialize(HWND handle, bool isFullScreen, const GraphicsConfiguration& configs) override;
    void cleanUp() override;

    void present() override { m_pSwapChain->Present(1, 0); }

    void submit(RendererT queue, RendererT* cmdLists, U32 numCmdLists) override;

    void createCommandList(CommandList** pList) override;
    void destroyCommandList(CommandList* pList) override;

    void createTexture2D() override;
    void createBuffer(Buffer** buffer, BufferDimension dimension, U32 width, U32 height = 1) override { CD3D11_BUFFER_DESC::}
    void destroyBuffer(Buffer* buffer) override;

    void createGraphicsPipelineState() override;
    void createComputePipelineState() override;
    void createRayTracingPipelineState() override;

    void createRayTracingPipelineState() override { 
        DEBUG("Ray Tracing pipeline not supported for D3D11 context!"); 
    }

private:

    ID3D11Device2* m_pDevice;
    ID3D11DeviceContext* m_pImmediateCtx;
    std::unordered_map<RendererT, GraphicsCommandListDeferredD3D11> m_deferredCmdLists;
    std::unordered_map<RendererT, GraphicsCommandListImmediateD3D11> m_immediateCmdLists;
    std::unordered_map<RendererT, ID3D11ShaderResourceView*> m_shaderResourceViews;
    std::unordered_map<RendererT, ID3D11RenderTargetView*> m_renderTargetViews;
    std::unordered_map<RendererT, ID3D11UnorderedAccessView*> m_unorderedAccessViews;
};
} // gfx