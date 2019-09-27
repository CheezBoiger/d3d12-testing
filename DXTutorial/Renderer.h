//
#pragma once

#include "WinConfigs.h"


namespace gfx {


struct GraphicsConfiguration 
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
    BUFFER_BIND_UNORDERED_ACCESS = (1 << 5)
};

typedef U32 BufferBindFlags;

enum BufferUsage
{
    BUFFER_USAGE_READBACK,
    BUFFER_USAGE_UPLOAD,
};

enum BufferDimension
{
    BUFFER_DIMENSION_BUFFER,
    BUFFER_DIMENSION_2D,
    BUFFER_DIMENSION_3D,
    BUFFER_DIMENSION_
};

typedef U64 RendererT;


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


struct Viewport
{
    R32 x, y, w, h;
    R32 mind, maxd;
};


struct Scissor
{
    U32 left, top, right, bottom;
};


class DescriptorTable : public GraphicsObject
{
public:
    virtual ~DescriptorTable() { }

    virtual void setShaderResourceViews() { }
    virtual void setUnorderedAccessViews() { }
    virtual void setConstantBuffers() { }
};


class RenderPass : public GraphicsObject
{
public:
    virtual void setRenderTargets() { }
    virtual void setDepthStencil() { }

};


class Buffer : public GraphicsObject
{
public:

};


class CommandList : public GraphicsObject
{
public:
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

    virtual void setPipelineStateObject() { }
    virtual void setRenderPass(RenderPass* pass) { }
    virtual void dispatch(U32 x, U32 y, U32 z) { }
    virtual void setVertexBuffers(Buffer** buffers, U32 vertexBufferCount) { }
    virtual void setIndexBuffer(Buffer** buffer) { }
    virtual void close() { }
    virtual void setViewports(Viewport* pViewports, U32 viewportCount) { }
    virtual void setScissors(Scissor* pScissors, U32 scissorCount) { }
    virtual void setDescriptorTables(DescriptorTable** pTables, U32 tableCount) { }
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
                            const GraphicsConfiguration& configs) { }
    virtual void cleanUp() { }

    virtual void adjust(GraphicsConfiguration& config) { }

    virtual void submit(RendererT queue, RendererT* cmdList, U32 numCmdLists) { }

    virtual void present() { }

    virtual void signalFence(RendererT queue, HANDLE fence) { }

    virtual void createBuffer(Buffer** buffer, 
                              BufferUsage usage,
                              BufferBindFlags binds, 
                              BufferDimension dimension, 
                              U32 width, 
                              U32 height = 1,
                              U32 depth = 1,
                              DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN) { }
    virtual void createTexture2D() { }
    virtual void createVertexBuffer() { }
    virtual void createIndexBuffer() { }
    virtual void createQueue() { }
    virtual void createGraphicsPipelineState() { }
    virtual void createComputePipelineState() { }
    virtual void createRayTracingPipelineState() { }
    virtual void createRenderPass(RenderPass** pPass) { }

    virtual void destroyBuffer(Buffer* buffer) { }
    virtual void destroyCommandList(CommandList* pCmdList) { }
    virtual void destoryRenderPass(RenderPass pPass) { }

    virtual void createCommandList(CommandList** pList) { }

protected:

    IDXGISwapChain1* m_pSwapChain;
};

} // gfx