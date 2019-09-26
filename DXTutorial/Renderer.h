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


enum BufferUsage
{
    BUFFER_USAGE_CONSTANT_BUFFER,
    BUFFER_USAGE_TEXTURE_2D,
    BUFFER_USAGE_TEXTURE_3D,
    BUFFER_USAGE_UNORDERED_ACCESS_VIEW
};

class VertexBuffer
{

};

class IndexBuffer
{

};

class DescriptorTable 
{
public:
    virtual ~DescriptorTable() { }


    virtual void setShaderResourceViews() { } 
    virtual void setUnorderedAccessViews() { }
    virtual void setConstantBuffers() { }
};


class RenderPass
{
public:
    virtual void setRenderTargets();
    virtual void setDepthStencil();
    
};

typedef U64 RendererT;

class AbstractGraphicsObject
{
    static RendererT assignmentOperator;
public:
    AbstractGraphicsObject()
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

class CommandList : public AbstractGraphicsObject
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
    virtual void setVertexBuffers(VertexBuffer* buffers, U32 vertexBufferCount) { }
    virtual void setIndexBuffer(IndexBuffer* buffer) { }
    virtual void close() { }
    virtual void setViewports(Viewport* pViewports, U32 viewportCount) { }
    virtual void setScissors(Scissor* pScissors, U32 scissorCount) { }
    virtual void setDescriptorTables(DescriptorTable** pTables, U32 tableCount) { }
};

class Buffer : public AbstractGraphicsObject
{
public:

};

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
    virtual void present() { m_pSwapChain->Present(1, 0); }
    virtual void signalFence(RendererT queue, HANDLE fence) { }

    virtual void beginFrame() { /* Frame wait for cpu fence. */ }
    virtual void endFrame() { /* Submit the job to the gpu queue. */ } 

    virtual void createBuffer(Buffer** pBuf) { }
    virtual void createTexture2D() { }
    virtual void createQueue() { }
    virtual void createGraphicsPipelineState() { }
    virtual void createComputePipelineState() { }
    virtual void createRayTracingPipelineState() { }
    virtual void createRenderPass(RenderPass** pPass) { }

    virtual void destroyCommandList(CommandList* pCmdList) { }
    virtual void destoryRenderPass(RenderPass pPass) { }

    virtual void createCommandList(CommandList** pList) { }

protected:

    IDXGISwapChain1* m_pSwapChain;
};

} // gfx