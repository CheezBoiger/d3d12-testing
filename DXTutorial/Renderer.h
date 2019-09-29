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


enum BufferBind
{
    BUFFER_BIND_CONSTANT_BUFFER = (1 << 0),
    BUFFER_BIND_RENDER_TARGET = (1 << 1),
    BUFFER_BIND_SHADER_RESOURCE = (1 << 2),
    BUFFER_BIND_VERTEX_BUFFER = (1 << 3),
    BUFFER_BIND_INDEX_BUFFER = (1 << 4),
    BUFFER_BIND_UNORDERED_ACCESS = (1 << 5),
    BUFFER_BIND_DEPTH_STENCIL = (1 << 6)
};

typedef U32 BufferBindFlags;

enum BufferUsage
{
    // Gpu read back from device memory to system memory.
    BUFFER_USAGE_GPU_TO_CPU,
    // Cpu upload from system memory to device memory. This is fast data transfer.
    BUFFER_USAGE_CPU_TO_GPU,
};

enum BufferDimension
{
    BUFFER_DIMENSION_BUFFER,
    BUFFER_DIMENSION_2D,
    BUFFER_DIMENSION_3D,
    BUFFER_DIMENSION_2D_ARRAY,
    BUFFER_DIMENSION_3D_ARRAY,
    BUFFER_DIMENSION_TEXTURE_CUBE,
    BUFFER_DIMENSION_TEXTURE_CUBE_ARRAY
};

enum CommandListRecordUsage
{
  COMMAND_LIST_RECORD_USAGE_SIMULTANEOUS,
  COMMAND_LIST_RECORD_USAGE_ONE_TIME_ONLY
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
class GraphicsObject
{
    static RendererT assignmentOperator;
public:
    GraphicsObject()
        : m_uuid(++assignmentOperator) { }

    RendererT getUUID() const { return m_uuid; }
private:
    RendererT m_uuid;
};


class Fence : public GraphicsObject
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


class Buffer : public GraphicsObject {
 public:
  virtual void* map(U64 start, U64 sz) { return nullptr; }
  virtual void unmap(U64 start, U64 sz) {}
};


class DescriptorTable : public GraphicsObject
{
public:
    virtual ~DescriptorTable() { }
    virtual void setShaderResourceViews(Buffer** buffers, U32 bufferCount) { }
    virtual void setUnorderedAccessViews(Buffer** buffers, U32 bufferCount) { }
    virtual void setConstantBuffers(Buffer** buffers, U32 bufferCount) { }
    virtual void finalize() { }
};


class GraphicsPipeline : public GraphicsObject
{
public:
};


class ComputePipeline : public GraphicsObject
{
public:
};


class RayTracingPipeline : public GraphicsObject
{
public:
};

class TargetView : public GraphicsObject
{
};

typedef TargetView RenderTargetView;
typedef TargetView DepthStencilView;
typedef TargetView ShaderResourceView;
typedef TargetView UnorderedAccessView;
typedef TargetView VertexBufferView;
typedef TargetView IndexBufferView;

class RenderPass : public GraphicsObject
{
public:
    virtual void setRenderTargets(RenderTargetView** pRenderTargets, U32 renderTargetCount) { }
    virtual void setDepthStencil(DepthStencilView* pDepthStencil) { }
    virtual void finalize() { }
};


class CommandList : public GraphicsObject
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
    virtual void setDescriptorTables(DescriptorTable** pTables, U32 tableCount) { }
    virtual void clearRenderTarget(RenderTargetView* rtv, R32* rgba, U32 numRects, RECT* rects) { }
    virtual void clearDepthStencil(DepthStencilView* dsv) { }
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

    virtual void createBuffer(Buffer** buffer,
                              BufferUsage usage,
                              BufferBindFlags binds, 
                              BufferDimension dimension, 
                              U32 width, 
                              U32 height = 1,
                              U32 depth = 1,
                              U32 structureByteStride = 1,
                              DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN,
                              const TCHAR* debugName = nullptr) { }

    virtual void createTexture2D() { }
    virtual void createQueue() { }
    virtual void createRenderTargetView(RenderTargetView** rtv, Buffer* buffer) { }
    virtual void createUnorderedAccessView(UnorderedAccessView** uav, Buffer* buffer) { }
    virtual void createShaderResourceView(ShaderResourceView** srv, 
                                          Buffer* buffer, 
                                          U32 firstElement, 
                                          U32 numElements) { }
    virtual void createDepthStencilView(DepthStencilView** dsv, Buffer* buffer) { }
    virtual void createGraphicsPipelineState(GraphicsPipeline** pipline) { }
    virtual void createComputePipelineState(ComputePipeline** pipeline) { }
    virtual void createRayTracingPipelineState() { }
    virtual void createVertexBufferView(VertexBufferView** view,
                                        Buffer* buffer, 
                                        U32 vertexStride, 
                                        U32 bufferSzBytes) { }
    virtual void createIndexBufferView(IndexBufferView** view) { }
    virtual void createRenderPass(RenderPass** pPass, 
                                  U32 rtvSize, 
                                  B32 hasDepthStencil) { }

    virtual void destroyBuffer(Buffer* buffer) { }
    virtual void destroyCommandList(CommandList* pCmdList) { }
    virtual void destroyRenderPass(RenderPass* pPass) { }

    virtual RendererT getSwapchainQueue() { return 0; }

    virtual void createCommandList(CommandList** pList, CommandListRecordUsage usage) { }

    virtual RenderPass* getBackbufferRenderPass() { return nullptr; }
    virtual Fence* getSwapchainFence() { return nullptr; }
    virtual RenderTargetView* getSwapchainRenderTargetVew() { return nullptr; }

protected:

    IDXGISwapChain1* m_pSwapChain;
};

} // gfx