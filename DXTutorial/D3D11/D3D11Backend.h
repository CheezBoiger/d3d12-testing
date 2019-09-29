

#pragma once

#include "../Renderer.h"

#include "CommonsD3D11.h"
#include "CommandListD3D11.h"

#include <vector>
#include <unordered_map>

namespace gfx {

struct BufferD3D11 : public Buffer
{
    U32 _width;
    U32 _height;
    U32 _depth;
    BufferDimension _dimension;
    DXGI_FORMAT _format;
    BufferBindFlags _flags;
};

struct VertexBufferViewD3D11 : public TargetView
{
  RendererT _buffer;
  U32 _stride;
};

class D3D11Backend : public BackendRenderer
{
public:

    void initialize(HWND handle, bool isFullScreen, const GpuConfiguration& configs) override;
    void cleanUp() override { }

    void present() override { m_pSwapChain->Present(1, 0); }

    void submit(RendererT queue,  CommandList** cmdLists, U32 numCmdLists) override { }

    void createCommandList(CommandList** pList,
                           CommandListRecordUsage usage) override { }
    void destroyCommandList(CommandList* pList) override { }

    void createTexture2D() override { }
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
    virtual void createVertexBufferView(VertexBufferView** view,
                                        Buffer* buffer, 
                                        U32 vertexStride, 
                                        U32 bufferSzBytes) override { }
    virtual void createIndexBufferView(IndexBufferView** view) override { }
    void destroyBuffer(Buffer* buffer) override { }

    void createGraphicsPipelineState(GraphicsPipeline** pipeline) override { }
    void createComputePipelineState(ComputePipeline** pipeline) override { }

    void createRayTracingPipelineState() override { 
        DEBUG("Ray Tracing pipeline not supported for D3D11 context!"); 
    }

    ID3D11Buffer* getBuffer(RendererT buff) {
      return m_buffers[buff];
    }

private:

    IDXGIFactory2* createFactory();
    void createDevice(IDXGIFactory2* pFactory);

    ID3D11Device2* m_pDevice;
    ID3D11DeviceContext* m_pImmediateCtx;
    std::unordered_map<RendererT, GraphicsCommandListDeferredD3D11> m_deferredCmdLists;
    std::unordered_map<RendererT, GraphicsCommandListImmediateD3D11> m_immediateCmdLists;
    std::unordered_map<RendererT, ID3D11Buffer*> m_buffers;
    std::unordered_map<RendererT, ID3D11RenderTargetView*> m_renderTargetViews;
    std::unordered_map<RendererT, ID3D11ShaderResourceView*> m_shaderResourceViews;
    std::unordered_map<RendererT, ID3D11DepthStencilView*> m_depthStencilViews;
    std::unordered_map<RendererT, ID3D11UnorderedAccessView*> m_unorderedAccessViews;
};
} // gfx