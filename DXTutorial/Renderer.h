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

typedef U64 RendererT;

class AbstractGraphicsObject
{
public:
    RendererT getUUID() const { return m_uuid; }
    void assignUUID(RendererT id) { m_uuid = id; }
private:
    RendererT m_uuid;
};

class CommandList : public AbstractGraphicsObject
{
public:
    CommandList() { }
    virtual ~CommandList() { }

    virtual void reset() { }
    virtual void drawIndexedInstanced() { }
    virtual void drawInstanced() { }
    virtual void setPipelineStateObject() { }
    virtual void setRenderTargets() { }
    virtual void shaderResourceViews() { }
    virtual void dispatch() { }
    virtual void setVertexBuffers() { }
    virtual void setIndexBuffer() { }
    virtual void close() { }
    virtual void setViewports() { }
    virtual void setScissors() { }
};

class Buffer : public AbstractGraphicsObject
{
public:

};

class BackendRenderer 
{
    static RendererT assignmentOperator;
public:

    virtual ~BackendRenderer() { }

    virtual void initialize(HWND hwnd, const GraphicsConfiguration& configs) { }
    virtual void cleanUp() { }

    virtual void adjust(GraphicsConfiguration& config) { }

    virtual void submit(RendererT queue, RendererT* cmdList, U32 numCmdLists) { }
    virtual void present() {  }
    virtual void signalFence(RendererT queue, HANDLE fence) { }

    virtual void beginFrame() { }
    virtual void endFrame() { } 

    void createBuffer(Buffer** pBuf) { onCreateBuffer(pBuf, assignmentOperator++); }
    void createTexture2D() { }
    void createQueue() { }
    void createGraphicsPipelineState() { }
    void createComputePipelineState() { }
    void createRayTracingPipelineState() { }

    void createCommandList(CommandList** pList) { onCreateCommandList(pList, assignmentOperator++); }

protected:
    virtual void onCreateCommandList(CommandList** pList, RendererT assignedUUID) = 0;
    virtual void onCreateBuffer(Buffer** pBuf, RendererT assignedUUID) = 0;
    virtual void onCreateGraphicsPipelineState(RendererT assignedUUID) = 0;
    virtual void onCreateComputePipelineState(RendererT assignedUUID) = 0;
private:
};

} // gfx