//
#pragma once

#include "WinConfigs.h"


namespace gfx {


struct GpuConfiguration 
{
    B32 _enableVSync;
    B32 _windowed;
    U32 _renderWidth;
    U32 _renderHeight;
    U32 _desiredBuffers;
};


enum ResourceBind
{
    RESOURCE_BIND_CONSTANT_BUFFER = (1 << 0),
    RESOURCE_BIND_RENDER_TARGET = (1 << 1),
    RESOURCE_BIND_SHADER_RESOURCE = (1 << 2),
    RESOURCE_BIND_VERTEX_BUFFER = (1 << 3),
    RESOURCE_BIND_INDEX_BUFFER = (1 << 4),
    RESOURCE_BIND_UNORDERED_ACCESS = (1 << 5),
    RESOURCE_BIND_DEPTH_STENCIL = (1 << 6)
};


enum ShaderVisibility {
  SHADER_VISIBILITY_VERTEX = 0x1,
  SHADER_VISIBILITY_HULL = 0x2,
  SHADER_VISIBILITY_DOMAIN = 0x4,
  SHADER_VISIBILITY_GEOMETRY = 0x8,
  SHADER_VISIBILITY_PIXEL = 0x10,
  SHADER_VISIBILITY_END,
  SHADER_VISIBILITY_ALL = 0x1F,
};

typedef U32 ShaderVisibilityFlags;
typedef U32 ResourceBindFlags;

enum ResourceUsage
{
    // Default usage.
    RESOURCE_USAGE_DEFAULT,
    // Gpu read back from device memory to system memory.
    RESOURCE_USAGE_GPU_TO_CPU,
    // Cpu upload from system memory to device memory. This is fast data transfer.
    RESOURCE_USAGE_CPU_TO_GPU,
};

enum ResourceDimension
{
    RESOURCE_DIMENSION_BUFFER,
    RESOURCE_DIMENSION_2D,
    RESOURCE_DIMENSION_3D,
    RESOURCE_DIMENSION_2D_ARRAY,
    RESOURCE_DIMENSION_3D_ARRAY,
    RESOURCE_DIMENSION_TEXTURE_CUBE,
    RESOURCE_DIMENSION_TEXTURE_CUBE_ARRAY
};

typedef U64 RendererT;

/*
  Graphics objects may have to update, or reinitialize during runtime (ex. window resize,
  minimize/maximize, graphics configurations from the user...) which will cause any currently
  bound objects to be become stale. To avoid having to re iniitalize every pointer to a gpu handle
  in this event, we simply track through a guuid/key system, and lookup the gpu handle. This way, our
  code remains clean, and we don't have to check for these events, other than performing one function call
  to reinit these gpu handles.
*/
class GPUObject
{
    static RendererT assignmentOperator;
public:
    GPUObject()
        : m_uuid(++assignmentOperator) { }

    RendererT getUUID() const { return m_uuid; }
private:
    RendererT m_uuid;
};


class Resource : public GPUObject
{
public:
  ResourceDimension _dimension;
  ResourceUsage _usage;
  ResourceBindFlags _bindFlags;
  Resource(ResourceDimension dimension, ResourceUsage usage, ResourceBindFlags flags) 
    : _dimension(dimension)
    , _usage(usage)
    , _bindFlags(flags) { }
  virtual void* map(U64 start, U64 sz) { return nullptr; }
  virtual void unmap(U64 start, U64 sz) { }
};


class Fence : public GPUObject
{
};


struct Viewport
{
    R32 x, y, w, h;
    R32 mind, maxd;
};


struct Scissor
{
    U32 left, top, right, bottom;
};


class Sampler : public GPUObject 
{
};


class DescriptorTable : public GPUObject
{
public:
    virtual ~DescriptorTable() { }
    virtual void setShaderResourceViews(Resource** resources, U32 bufferCount) { }
    virtual void setUnorderedAccessViews(Resource** resources, U32 bufferCount) { }
    virtual void setConstantBuffers(Resource** buffer, U32 bufferCount) { }
    virtual void setSamplers(Sampler** samplers, U32 samplerCount) { }
    virtual void finalize(ShaderVisibilityFlags visibleFlags) { }
    virtual void update() { }
};


class GraphicsPipeline : public GPUObject
{
public:
};


class ComputePipeline : public GPUObject
{
public:
};


class RayTracingPipeline : public GPUObject
{
public:
};


class TargetView : public GPUObject
{
};


typedef TargetView RenderTargetView;
typedef TargetView DepthStencilView;
typedef TargetView ShaderResourceView;
typedef TargetView UnorderedAccessView;
typedef TargetView VertexBufferView;
typedef TargetView IndexBufferView;

class RenderPass : public GPUObject
{
public:
    virtual void setRenderTargets(RenderTargetView** pRenderTargets, U32 renderTargetCount) { }
    virtual void setDepthStencil(DepthStencilView* pDepthStencil) { }
    virtual void finalize() { }
};


class CommandList : public GPUObject
{
public:
    static const U64 kSwapchainRenderTargetId = 0xffffffffffffffff;

    CommandList() { }
    virtual ~CommandList() { }

    virtual void init() { }
    virtual void destroy() { }

    virtual void reset() { }

    virtual void drawIndexedInstanced(U32 indexCountPerInstance, 
                                      U32 instanceCount, 
                                      U32 startIndexLocation, 
                                      U32 baseVertexLocation, 
                                      U32 startInstanceLocation) { }

    virtual void drawInstanced(U32 vertexCountPerInstance, 
                               U32 instanceCount, 
                               U32 startVertexLocation, 
                               U32 startInstanceLocation) { }

    virtual void setGraphicsPipeline(GraphicsPipeline* pPipeline) { }
    virtual void setComputePipeline(ComputePipeline* pPipeline) { }
    virtual void setRayTracingPipeline(RayTracingPipeline* pPipeline) { }
    virtual void setAccelerationStructure() { }
    virtual void setRenderPass(RenderPass* pass) { }
    virtual void dispatch(U32 x, U32 y, U32 z) { }
    virtual void setVertexBuffers(VertexBufferView** vbvs, U32 vertexBufferCount) { }
    virtual void setIndexBuffer(IndexBufferView* buffer) { }
    virtual void close() { }
    virtual void setViewports(Viewport* pViewports, U32 viewportCount) { }
    virtual void setScissors(Scissor* pScissors, U32 scissorCount) { }
    virtual void setDescriptorTables(DescriptorTable** pTables, U32 tableCount, B32 compute) { }
    virtual void clearRenderTarget(RenderTargetView* rtv, R32* rgba, U32 numRects, RECT* rects) { }
    virtual void clearDepthStencil(DepthStencilView* dsv) { }

    B32 isRecording() const { return _isRecording; }

protected:
    B32 _isRecording;
};

/*
    Behind every great renderer is a powerful engine that handles the translation between
    front end rendering, to native gpu calls. The backend renderer has an important duty to ensure
    that there is a fine line between getting the visuals needed, and getting the visuals to work on anything,
    this module is responsible for the low level work. Settting up a Rendering engine in this way will allow for
    better handling of support for multiple Graphics APIs, along with focusing on the important aspects of simplicity
    between front end work, and back end work.

    You can even set up a null backend renderer, or even a software renderer.
*/
class BackendRenderer 
{
public:
    static const U64 kGraphicsQueueId = 0;

    virtual ~BackendRenderer() { }

    virtual void initialize(HWND hwnd, 
                            bool isFullScreen, 
                            const GpuConfiguration& configs) { }
    virtual void cleanUp() { }

    virtual void adjust(GpuConfiguration& config) { }

    virtual void submit(RendererT queue,  CommandList** cmdLists, U32 numCmdLists) { }

    virtual void present() { }

    virtual void signalFence(RendererT queue, Fence* fence) { }
    virtual void waitFence(Fence* fence) { }

    virtual void createBuffer(Resource** buffer,
                              ResourceUsage usage,
                              ResourceBindFlags binds,
                              U32 widthBytes,
                              U32 structureByteStride = 0,
                              const TCHAR* debugName = nullptr) { }

    virtual void createTexture(Resource** texture,
                               ResourceDimension dimension,
                               ResourceUsage usage,
                               ResourceBindFlags binds,
                               DXGI_FORMAT format,
                               U32 width,
                               U32 height,
                               U32 depth = 1,
                               U32 structureByteStride = 0,
                               const TCHAR* debugName = nullptr) { }
    virtual void createQueue() { }
    virtual void createRenderTargetView(RenderTargetView** rtv, Resource* texture) { }
    virtual void createUnorderedAccessView(UnorderedAccessView** uav, Resource* texture) { }
    virtual void createShaderResourceView(ShaderResourceView** srv, 
                                          Resource* resource, 
                                          U32 firstElement, 
                                          U32 numElements) { }
    virtual void createDepthStencilView(DepthStencilView** dsv, Resource* texture) { }
    virtual void createGraphicsPipelineState(GraphicsPipeline** pipline) { }
    virtual void createComputePipelineState(ComputePipeline** pipeline) { }
    virtual void createRayTracingPipelineState() { }
    virtual void createVertexBufferView(VertexBufferView** view,
                                        Resource* buffer, 
                                        U32 vertexStride, 
                                        U32 bufferSzBytes) { }
    virtual void createIndexBufferView(IndexBufferView** view) { }
    virtual void createDescriptorTable(DescriptorTable** table) { }
    virtual void createRenderPass(RenderPass** pPass, 
                                  U32 rtvSize, 
                                  B32 hasDepthStencil) { }

    virtual void destroyResource(Resource* resource) { }
    virtual void destroyCommandList(CommandList* pCmdList) { }
    virtual void destroyRenderPass(RenderPass* pPass) { }

    virtual void createSampler(Sampler** sampler) { }
    virtual void destroySampler(Sampler* sampler) { }
    virtual void destroyDescriptorTable(DescriptorTable* table) { }

    virtual RendererT getSwapchainQueue() { return 0; }

    virtual void createCommandList(CommandList** pList) { }

    virtual RenderPass* getBackbufferRenderPass() { return nullptr; }
    virtual Fence* getSwapchainFence() { return nullptr; }
    virtual RenderTargetView* getSwapchainRenderTargetView() { return nullptr; }

protected:

    IDXGISwapChain1* m_pSwapChain;
};

} // gfx