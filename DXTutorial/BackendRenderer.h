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


enum ShaderVisibility 
{
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


enum ClearFlag
{
  CLEAR_FLAG_DEPTH = (1 << 0),
  CLEAR_FLAG_STENCIL = (1 << 1)
};


typedef U32 ClearFlags;
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


class CommandQueue : public GPUObject
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
    virtual void finalize() { }
    virtual void update() { }
};


struct PipelineLayout
{
  U32 numConstantBuffers;
  U32 numSamplers;
  U32 numUnorderedAcessViews;
  U32 numShaderResourceViews;
};

class RootSignature : public GPUObject
{
public:
  virtual void initialize(ShaderVisibilityFlags visibleFlags,
                          PipelineLayout* pLayouts, 
                          U32 numLayouts) { }
};


struct ShaderByteCode 
{
  void* _pByteCode;
  size_t _szBytes;
};


enum PrimitiveTopology 
{
  PRIMITIVE_TOPOLOGY_POINTS,
  PRIMITIVE_TOPOLOGY_LINES,
  PRIMITIVE_TOPOLOGY_TRIANGLES,
  PRIMITIVE_TOPOLOGY_PATCHES
};


enum CullMode
{
  CULL_MODE_BACK,
  CULL_MODE_FRONT,
  CULL_MODE_NONE
};


enum FillMode
{
  FILL_MODE_SOLID,
  FILL_MODE_WIREFRAME
};


struct RasterizationStateInfo
{
  B32 _antialiasedLinesEnable;
  B32 _conservativeRasterizationEnable;
  CullMode _cullMode;
  FillMode _fillMode;
  I32 _depthBias;
  B32 _depthClipEnable;
  B32 _multisampleEnable;
  R32 _depthBiasClamp;
  U32 _forcedSampleCount;
  B32 _frontCounterClockwise;
  R32 _slopedScaledDepthBias;
};


enum StencilOp 
{
  STENCIL_OP_KEEP,
  STENCIL_OP_ZERO,
  STENCIL_OP_REPLACE,
  STENCIL_OP_INCR_SAT,
  STENCIL_OP_DECR_SAT,
  STENCIL_OP_INVERT,
  STENCIL_OP_INCR,
  STENCIL_OP_DECR
};


enum ComparisonFunc
{
  COMPARISON_FUNC_NEVER,
  COMPARISON_FUNC_LESS,
  COMPARISON_FUNC_EQUAL,
  COMPARISON_FUNC_LESS_EQUAL,
  COMPARISON_FUNC_GREATER,
  COMPARISON_FUNC_NOT_EQUAL,
  COMPARISON_FUNC_GREATER_EQUAL,
  COMPARISON_FUNC_ALWAYS
};


enum DepthWriteMask
{
  DEPTH_WRITE_MASK_ZERO,
  DEPTH_WRITE_MASK_ALL
};


struct DepthStencilOpDesc 
{
  ComparisonFunc _stencilFunc;
  StencilOp _stencilFailOp;
  StencilOp _stencilDepthFailOp;
  StencilOp _stencilPassOp;
};


struct DepthStencilStateInfo
{
  DepthWriteMask _depthWriteMask;
  ComparisonFunc _depthFunc;
  B8 _depthEnable;
  B8 _stencilEnable;
  U8 _stencilReadMask;
  U8 _stencilWriteMask;
  DepthStencilOpDesc _frontFace;
  DepthStencilOpDesc _backFace;
};


enum Blend
{
  BLEND_ZERO = 1,
  BLEND_ONE,
  BLEND_SRC_COLOR,
  BLEND_INV_SRC_COLOR,
  BLEND_SRC_ALPHA,
  BLEND_INV_SRC_ALPHA,
  BLEND_DEST_ALPHA,
  BLEND_INV_DEST_ALPHA,
  BLEND_DEST_COLOR,
  BLEND_INV_DEST_COLOR,
  BLEND_SRC_ALPHA_SAT,
  BLEND_BLEND_FACTOR,
  BLEND_INV_BLEND_FACTOR,
  BLEND_SRC1_COLOR,
  BLEND_INV_SRC1_COLOR,
  BLEND_SRC1_ALPHA,
  BLEND_INV_SRC1_ALPHA
};


enum BlendOp
{
  BLEND_OP_ADD,
  BLEND_OP_SUBTRACT,
  BLEND_OP_REV_SUBTRACT,
  BLEND_OP_MIN,
  BLEND_OP_MAX
};


enum LogicOp
{
  LOGIC_OP_CLEAR,
  LOGIC_OP_SET,
  LOGIC_OP_COPY,
  LOGIC_OP_COPY_INVERTED,
  LOGIC_OP_NOOP,
  LOGIC_OP_INVERT,
  LOGIC_OP_AND,
  LOGIC_OP_NAND,
  LOGIC_OP_OR,
  LOGIC_OP_NOR,
  LOGIC_OP_XOR,
  LOGIC_OP_EQUIV,
  LOGIC_OP_AND_REVERSE,
  LOGIC_OP_AND_INVERTED,
  LOGIC_OP_OR_REVERSE,
  LOGIC_OP_OR_INVERTED
};


enum ColorWriteEnable
{
  COLOR_WRITE_ENABLE_RED = 1,
  COLOR_WRITE_ENABLE_GREEN = 2,
  COLOR_WRITE_ENABLE_BLUE = 4,
  COLOR_WRITE_ENABLE_ALPHA = 8,
  COLOR_WRITE_ENABLE_ALL =
      (((COLOR_WRITE_ENABLE_RED | COLOR_WRITE_ENABLE_GREEN) |
        COLOR_WRITE_ENABLE_BLUE) |
        COLOR_WRITE_ENABLE_ALPHA)
};


struct RenderTargetBlend
{
  B8 _blendEnable;
  B8 _logicOpEnable;
  Blend _srcBlend;
  Blend _dstBlend;
  BlendOp _blendOp;
  Blend _srcBlendAlpha;
  Blend _dstBlendAlpha;
  BlendOp _blendOpAlpha;
  LogicOp _logicOp;
  U8 _renderTargetWriteMask;
};


struct BlendStateInfo
{
  B8 _alphaToCoverageEnable;
  B8 _independentBlendEnable;
  RenderTargetBlend _renderTargets[8];
};


enum InputClassification
{
  INPUT_CLASSIFICATION_PER_VERTEX,
  INPUT_CLASSIFICATION_PER_INSTANCE
};


enum IBCutValue
{
  IB_CUT_VALUE_DISABLED,
  IB_CUT_VALUE_CUT_0xFFFF,
  IB_CUT_VALUE_CUT_0xFFFFFFFF
};


struct InputElementInfo
{
  const CHAR* _semanticName;
  U32 _semanticIndex;
  DXGI_FORMAT _format;
  U32 _inputSlot;
  U32 _alignedByteOffset;
  InputClassification _classification;
  U32 _instanceDataStepRate;
};


struct InputLayoutInfo
{
  InputElementInfo* _pInputElements;
  U32 _elementCount;
};


struct GraphicsPipelineInfo 
{
  PrimitiveTopology _topology;
  DXGI_FORMAT _dsvFormat;
  U32 _numRenderTargets;
  U32 _sampleMask;
  DXGI_FORMAT _rtvFormats[8];
  ShaderByteCode _vertexShader;
  ShaderByteCode _hullShader;
  ShaderByteCode _domainShader;
  ShaderByteCode _geometryShader;
  ShaderByteCode _pixelShader;

  RootSignature* _pRootSignature;
  RasterizationStateInfo _rasterizationState;
  DepthStencilStateInfo _depthStencilState;
  BlendStateInfo _blendState;
  IBCutValue _ibCutValue;
  InputLayoutInfo _inputLayout;
};


struct ComputePipelineInfo 
{
  ShaderByteCode _computeShader;
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
    virtual void setAccelerationStructure(Resource* pAccelerationStructure) { }
    virtual void setRenderPass(RenderPass* pass) { }
    virtual void dispatch(U32 x, U32 y, U32 z) { }
    virtual void setVertexBuffers(U32 startSlot, VertexBufferView** vbvs, U32 vertexBufferCount) { }
    virtual void setGraphicsRootSignature(RootSignature* pRootSignature) { }
    virtual void setComputeRootSignature(RootSignature* pRootSignature) { }
    virtual void setIndexBuffer(IndexBufferView* buffer) { }
    virtual void setGraphicsRootDescriptorTable(U32 rootParameterIndex, DescriptorTable* pTable) { }
    virtual void setComputeRootDescriptorTable(U32 rootParameterIndex, DescriptorTable* pTable) { }
    virtual void setComputeRootConstantBufferView(U32 rootParameterIndex, Resource* pConstantBuffer) { }
    virtual void setComputeRootShaderResourceView(U32 rootParameterIndex, Resource* pShaderResourceView) { }
    virtual void setGraphicsRootConstantBufferView(U32 rootParameterIndex, Resource* pConstantBuffer) { }
    virtual void setGraphicsRootShaderResourceView(U32 rootParameterIndex, Resource pShaderResourceView) { }
    virtual void close() { }
    virtual void setViewports(Viewport* pViewports, U32 viewportCount) { }
    virtual void setScissors(Scissor* pScissors, U32 scissorCount) { }
    virtual void setDescriptorTables(DescriptorTable** pTables, U32 tableCount) { }
    virtual void clearRenderTarget(RenderTargetView* rtv, R32* rgba, U32 numRects, RECT* rects) { }
    virtual void clearDepthStencil(DepthStencilView* dsv, 
                                   ClearFlags flags,
                                   R32 depth, 
                                   U8 stencil, 
                                   U32 numRects,
                                   const RECT* rects) {}
    virtual void copyResource(Resource* pDst, Resource* pSrc) { }

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
    virtual void createQueue(CommandQueue** ppQueue) { }
    virtual void createRenderTargetView(RenderTargetView** rtv, Resource* texture) { }
    virtual void createUnorderedAccessView(UnorderedAccessView** uav, Resource* texture) { }
    virtual void createShaderResourceView(ShaderResourceView** srv, 
                                          Resource* resource, 
                                          U32 firstElement, 
                                          U32 numElements) { }
    virtual void createDepthStencilView(DepthStencilView** dsv, Resource* texture) { }
    virtual void createGraphicsPipelineState(GraphicsPipeline** pipline,
                                             const GraphicsPipelineInfo* pInfo) { }
    virtual void createComputePipelineState(ComputePipeline** pipeline,
                                            const ComputePipelineInfo* pInfo) { }
    virtual void createRayTracingPipelineState(RayTracingPipeline** ppPipeline) { }

    // creates acceleration structure for hardware ray tracing.
    virtual void createAccelerationStructure(Resource** ppResource) { }
  
    // Machine learning operators for ml assisted rendering.
    virtual void createMLOperator() { }
    virtual void createBindingTable() { }
    virtual void createOperatorInitializer() { }

    virtual void createVertexBufferView(VertexBufferView** view,
                                        Resource* buffer, 
                                        U32 vertexStride, 
                                        U32 bufferSzBytes) { }
    virtual void createIndexBufferView(IndexBufferView** view,
                                       Resource* buffer,
                                       DXGI_FORMAT format,
                                       U32 szBytes) { }
    virtual void createDescriptorTable(DescriptorTable** table) { }
    virtual void createRenderPass(RenderPass** pPass, 
                                  U32 rtvSize, 
                                  B32 hasDepthStencil) { }
    virtual void createRootSignature(RootSignature** pRootSignature) { }

    virtual void destroyRootSignature(RootSignature* pRootSig) { }
    virtual void destroyResource(Resource* resource) { }
    virtual void destroyCommandList(CommandList* pCmdList) { }
    virtual void destroyRenderPass(RenderPass* pPass) { }

    virtual void createSampler(Sampler** sampler) { }
    virtual void destroySampler(Sampler* sampler) { }
    virtual void destroyDescriptorTable(DescriptorTable* table) { }

    virtual RendererT getSwapchainQueue() { return 0; }

    virtual void createCommandList(CommandList** pList) { }

    virtual RenderPass* getBackbufferRenderPass() { return nullptr; }
    virtual RenderTargetView* getSwapchainRenderTargetView() { return nullptr; }
    virtual void createFence(Fence** ppFence) { }
    virtual void destroyFence(Fence* pFence) { }

    bool isHardwareRaytracingCompatible() const { return m_hardwareRaytracingCompatible; }
    bool isHardwareMachineLearningCompatible() const { return m_harwareMachineLearningCompatible; }

protected:

    IDXGISwapChain1* m_pSwapChain;
    B32 m_hardwareRaytracingCompatible;
    B32 m_harwareMachineLearningCompatible;
};

} // gfx