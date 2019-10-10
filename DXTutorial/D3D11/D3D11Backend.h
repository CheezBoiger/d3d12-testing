

#pragma once

#include "../Renderer.h"

#include "CommonsD3D11.h"

#include <vector>
#include <unordered_map>

namespace gfx {


class D3D11Backend;


struct FrameResourceD3D11 {
  ID3D11Texture2D* _surface;
  TargetView _view;
};


struct GraphicsPipelineD3D11 : public GraphicsPipeline
{
    RendererT _vs           : 16; // verex shader.
    RendererT _hs           : 16; // hull shader
    RendererT _ds           : 16; // domain shader.
    RendererT _gs           : 16; // geometry shader
    RendererT _ps           : 16; // pixel shader
    RendererT _bs           : 16; // blend state.
    RendererT _depthstencil : 16; // depth stencil state.
};


struct BufferD3D11 : public Resource
{
    BufferD3D11(ResourceDimension dimension,
                ResourceUsage usage,
                ResourceBindFlags flags) 
    : Resource(dimension, usage, flags) { }
    virtual void* map(U64 start, U64 sz) override;
    virtual void unmap(U64 start, U64 sz) override;
    U32 _width;
    ResourceBindFlags _flags;
    D3D11Backend* _pBackend;
};


struct TextureD3D11 : public BufferD3D11
{
  TextureD3D11(ResourceDimension dimension, ResourceUsage usage, ResourceBindFlags flags)
    : BufferD3D11(dimension, usage, flags)
    { }
  DXGI_FORMAT _format;
  U32 _height;
  U32 _depth;
};


struct VertexBufferViewD3D11 : public TargetView
{
  RendererT _buffer;
  U32 _stride;
  U32 _szBytes;
};


struct IndexBufferViewD3D11 : public TargetView
{
  RendererT _buffer;
  DXGI_FORMAT _format;  
  U32 _szBytes;
};


class D3D11Backend : public BackendRenderer
{
public:

    void initialize(HWND handle, bool isFullScreen, const GpuConfiguration& configs) override;
    void cleanUp() override { }

    void present() override;

    void submit(RendererT queue,  CommandList** cmdLists, U32 numCmdLists) override;

    void createCommandList(CommandList** pList) override;
    void destroyCommandList(CommandList* pList) override { }

    void createTexture(Resource** texture,
                       ResourceDimension dimension,
                       ResourceUsage usage,
                       ResourceBindFlags binds,
                       DXGI_FORMAT format,
                       U32 width,
                       U32 height,
                       U32 depth = 1,
                       U32 structureByteStride = 0,
                       const TCHAR* debugName = nullptr) override;
    void createBuffer(Resource** buffer, 
                      ResourceUsage usage,
                      ResourceBindFlags binds,
                      U32 widthBytes, 
                      U32 structureByteStride = 0,
                      const TCHAR* debugName = nullptr) override;
    virtual void createVertexBufferView(VertexBufferView** view,
                                        Resource* buffer, 
                                        U32 vertexStride, 
                                        U32 bufferSzBytes) override;
    virtual void createIndexBufferView(IndexBufferView** view,
                                       Resource* buffer,
                                       DXGI_FORMAT format,
                                       U32 szBytes) override;
    void createRenderTargetView(RenderTargetView** rtv, Resource* buffer) override;
    void createDepthStencilView(DepthStencilView** ppDsv, Resource* buffer) override;
    void destroyResource(Resource* buffer) override { }
    void createDescriptorTable(DescriptorTable** table) override;

    void createRootSignature(RootSignature** ppRootSig) override;
    void destroyRootSignature(RootSignature* pRootSig) override { }
    void createGraphicsPipelineState(GraphicsPipeline** pipeline,
                                     const GraphicsPipelineInfo* pInfo) override;
    void createComputePipelineState(ComputePipeline** pipeline,
                                    const ComputePipelineInfo* pInfo) override { }
    void createShader(Shader** ppShader, ShaderType type, const ShaderByteCode* pBytecode) override;
    void destroyShader(Shader* pShader) override;

    void createRayTracingPipelineState(RayTracingPipeline** pPipeline) override { 
        DEBUG("Ray Tracing pipeline not supported for D3D11 context!"); 
    }

    ID3D11Resource* getResource(RendererT res) {
      return m_resources[res];
    }

    ID3D11DeviceContext* getImmediateCtx() { return m_pImmediateCtx; }

    ID3D11Device* getDevice() { return m_pDevice; }

    RendererT getSwapchainQueue() override { return kGraphicsQueueId; }

    RenderTargetView* getSwapchainRenderTargetView() override {
      return &m_frameResource._view;
    }

    ID3D11RenderTargetView* getRenderTargetView(RendererT uuid) {
      return m_renderTargetViews[uuid];
    }

    ID3D11DepthStencilView* getDepthStencilView(RendererT uuid) {
      return m_depthStencilViews[uuid];
    }

private:

    IDXGIFactory2* createFactory();
    void createDevice(IDXGIFactory2* pFactory);
    void createSwapchain(HWND hWnd, 
                         IDXGIFactory2* pFactory, 
                         U32 renderWidth,
                         U32 renderHeight, 
                         U32 desiredBuffers, 
                         B32 windowed);
    void queryFromSwapchain();

    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pImmediateCtx;
    std::unordered_map<RendererT, ID3D11Resource*> m_resources;
    std::unordered_map<RendererT, ID3D11RenderTargetView*> m_renderTargetViews;
    std::unordered_map<RendererT, ID3D11ShaderResourceView*> m_shaderResourceViews;
    std::unordered_map<RendererT, ID3D11DepthStencilView*> m_depthStencilViews;
    std::unordered_map<RendererT, ID3D11UnorderedAccessView*> m_unorderedAccessViews;

    // Pipeline values, use hashes to determine which are the same.
    std::unordered_map<RendererT, ID3D11VertexShader*> m_pVertexShaders;
    std::unordered_map<RendererT, ID3D11PixelShader*> m_pPixelShaders;
    std::unordered_map<RendererT, ID3D11GeometryShader*> m_pGeometryShaders;
    std::unordered_map<RendererT, ID3D11HullShader*> m_pHullShaders;
    std::unordered_map<RendererT, ID3D11DomainShader*> m_pDomainShaders;
    std::unordered_map<RendererT, ID3D11ComputeShader*> m_pComputeShaders;
    std::unordered_map<RendererT, ID3D11DepthStencilState*> m_pDepthStencilStates;

    FrameResourceD3D11 m_frameResource;
};

D3D11Backend* getBackendD3D11();
} // gfx