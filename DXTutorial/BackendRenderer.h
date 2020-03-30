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
    RESOURCE_DIMENSION_1D,
    RESOURCE_DIMENSION_2D,
    RESOURCE_DIMENSION_3D,
    RESOURCE_DIMENSION_1D_ARRAY,
    RESOURCE_DIMENSION_2D_ARRAY,
    RESOURCE_DIMENSION_TEXTURE_CUBE,
    RESOURCE_DIMENSION_TEXTURE_CUBE_ARRAY,
    RESOURCE_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE
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


struct ResourceMappingRange
{
    U64 _start;
    U64 _sz;
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
  virtual void* map(const ResourceMappingRange* pRange = nullptr) { return nullptr; }
  virtual void unmap(const ResourceMappingRange* pRange = nullptr) { }
};


enum BufferSrvFlags
{
    BUFFER_SRV_FLAGS_NONE,
    BUFFER_SRV_FLAGS_RAW = 0x1
};


struct BufferSRV
{
    U64 _firstElement;
    U32 _numElements;
    U32 _structureByteStride;
    BufferSrvFlags _flags;
};

struct Texture1DSRV
{
    U32 _mostDetailedMip;
    U32 _mipLevels;
    R32 _resourceMinLODClamp;
};

struct Texture1DArraySRV
{
    U32 _mostDetailedMip;
    U32 _mipLevels;
    U32 _firstArraySlice;
    U32 _arraySize;
    R32 _resourceMinLODClamp;
};

struct Texture2DSRV
{
    U32 _mostDetailedMip;
    U32 _mipLevels;
    U32 _planeSlice;
    R32 _resourceMinLODClamp;
};

struct Texture2DArraySRV
{
    U32 _mostDetailedMip;
    U32 _mipLevels;
    U32 _firstArraySlice;
    U32 _arraySize;
    U32 _planeSlice;
    R32 _resourceMinLodClamp;
};

struct Texture3DSRV
{
    U32 _mostDetailedMip;
    U32 _mipLevels;
    R32 _resourceMinLODClamp;
};

struct TextureCubeSRV
{
    U32 _mostDetailedMip;
    U32 _mipLevels;
    R32 _resourceMinLODClamp;
};

struct TextureCubeArraySRV
{
    U32 _mostDetailedMip;
    U32 _mipLevels;
    U32 _first2DArrayFace;
    U32 _numCubes;
    R32 _resourceMinLODClamp;
};

struct RayTracingAccelerationStructureSRV
{
    Resource* _location;
};

struct ShaderResourceViewDesc
{
    ResourceDimension _dimension;
    DXGI_FORMAT _format;
    union {
        BufferSRV _buffer;
        Texture1DSRV _texture1D;
        Texture1DArraySRV _texture1DArray;
        Texture2DSRV _texture2D;
        Texture2DArraySRV _texture2DArray;
        Texture3DSRV _texture3D;
        TextureCubeSRV _textureCube;
        TextureCubeArraySRV _textureCubeArray;
        RayTracingAccelerationStructureSRV _rayTracingAccelerationStructure;
    };
};

struct BufferRTV
{
    U64 _firstElement;
    U32 _numElements;
};

struct Texture1DRTV
{
    U32 _mipSlice;
};

struct Texture1DArrayRTV
{
    U32 _mipSlice;
    U32 _firstArraySlice;
    U32 _arraySize;
};

struct Texture2DRTV
{
    U32 _mipSlice;
    U32 _planeSlice;
};

struct Texture2DArrayRTV
{
    U32 _mipSlice;
    U32 _firstArraySlice;
    U32 _arraySize;
    U32 _planeSlice;
};

struct Texture3DRTV
{
    U32 _mipSlice;
    U32 _firstWSlice;
    U32 _wSize;
};

struct RenderTargetViewDesc
{
    ResourceDimension _dimension;
    DXGI_FORMAT _format;
    union {
        BufferRTV _buffer;
        Texture1DRTV _texture1D;
        Texture1DArrayRTV _texture1DArray;
        Texture2DRTV _texture2D;
        Texture2DArrayRTV _texture2DArray;
        Texture3DRTV _texture3D;
    };
};

enum DepthStencilFlags
{
    DEPTH_STENCIL_FLAG_NONE,
    DEPTH_STENCIL_FLAG_READ_ONLY_DEPTH = 0x1,
    DEPTH_STENCIL_FLAG_READ_ONLY_STENCIL = 0x2
};

struct Texture1DDSV
{
    U32 _mipSlice;
};

struct Texture1DArrayDSV
{
    U32 _mipSlice;
    U32 _firstArraySlice;
    U32 _arraySize;
};

struct Texture2DDSV
{
    U32 _mipSlice;
};

struct Texture2DArrayDSV
{
    U32 _mipSlice;
    U32 _firstArraySlice;
    U32 _arraySize;
};

struct DepthStencilViewDesc
{
    DXGI_FORMAT _format;
    ResourceDimension _dimension;
    U32 _flags;
    union {
        Texture1DDSV _texture1D;
        Texture1DArrayDSV _texture1DArray;
        Texture2DDSV _texture2D;
        Texture2DArrayDSV _texture2DArray;
    };
};


enum BufferUAVFlag
{
    BUFFER_UAV_FLAG_NONE,
    BUFFER_UAV_FLAG_RAW = 0x1
};

struct BufferUAV
{
    U64 _firstElement;
    U32 _numElements;
    U32 _structureByteStride;
    U64 _counterOffsetInBytes;
    U32 _flags;
};

struct Texture1DUAV
{
    U32 _mipSlice;
};

struct Texture1DArrayUAV
{
    U32 _mipSlice;
    U32 _firstArraySlice;
    U32 _arraySize;
};

struct Texture2DUAV
{
    U32 _mipSlice;
    U32 _planeSlice;
};

struct Texture2DArrayUAV
{
    U32 _mipSlice;
    U32 _firstArraySlice;
    U32 _arraySize;
    U32 _planeSlice;
};

struct Texture3DUAV
{
    U32 _mipSlice;
    U32 _firstWSlice;
    U32 _wSize;
};

struct UnorderedAccessViewDesc
{
    ResourceDimension _dimension;
    DXGI_FORMAT _format;
    union {
        BufferUAV _buffer;
        Texture1DUAV _texture1D;
        Texture1DArrayUAV _texture1DArray;
        Texture2DUAV _texture2D;
        Texture2DArrayUAV _texture2DArray;
        Texture3DUAV _texture3D; 
    };
};


class TargetView : public GPUObject
{
};


typedef TargetView RenderTargetView;
typedef TargetView DepthStencilView;
typedef TargetView ShaderResourceView;
typedef TargetView UnorderedAccessView;
typedef TargetView VertexBufferView;


class Fence : public GPUObject
{
};


enum CommandQueueType
{
    COMMAND_QUEUE_TYPE_DIRECT,
    COMMAND_QUEUE_TYPE_COMPUTE,
    COMMAND_QUEUE_TYPE_BUNDLE
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



enum SamplerAddressMode
{
    SAMPLER_ADDRESS_MODE_BORDER,
    SAMPLER_ADDRESS_MODE_CLAMP,
    SAMPLER_ADDRESS_MODE_MIRROR,
    SAMPLER_ADDRESS_MODE_MIRROR_ONCE,
    SAMPLER_ADDRESS_MODE_WRAP
};


enum SamplerFilter
{
        SAMPLER_FILTER_MIN_MAG_MIP_POINT	= 0,
        SAMPLER_FILTER_MIN_MAG_POINT_MIP_LINEAR	= 0x1,
        SAMPLER_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x4,
        SAMPLER_FILTER_MIN_POINT_MAG_MIP_LINEAR	= 0x5,
        SAMPLER_FILTER_MIN_LINEAR_MAG_MIP_POINT	= 0x10,
        SAMPLER_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x11,
        SAMPLER_FILTER_MIN_MAG_LINEAR_MIP_POINT	= 0x14,
        SAMPLER_FILTER_MIN_MAG_MIP_LINEAR	= 0x15,
        SAMPLER_FILTER_ANISOTROPIC	= 0x55,
        SAMPLER_FILTER_COMPARISON_MIN_MAG_MIP_POINT	= 0x80,
        SAMPLER_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR	= 0x81,
        SAMPLER_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x84,
        SAMPLER_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR	= 0x85,
        SAMPLER_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT	= 0x90,
        SAMPLER_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x91,
        SAMPLER_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT	= 0x94,
        SAMPLER_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR	= 0x95,
        SAMPLER_FILTER_COMPARISON_ANISOTROPIC	= 0xd5,
        SAMPLER_FILTER_MINIMUM_MIN_MAG_MIP_POINT	= 0x100,
        SAMPLER_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR	= 0x101,
        SAMPLER_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x104,
        SAMPLER_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR	= 0x105,
        SAMPLER_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT	= 0x110,
        SAMPLER_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x111,
        SAMPLER_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT	= 0x114,
        SAMPLER_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR	= 0x115,
        SAMPLER_FILTER_MINIMUM_ANISOTROPIC	= 0x155,
        SAMPLER_FILTER_MAXIMUM_MIN_MAG_MIP_POINT	= 0x180,
        SAMPLER_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR	= 0x181,
        SAMPLER_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x184,
        SAMPLER_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR	= 0x185,
        SAMPLER_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT	= 0x190,
        SAMPLER_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x191,
        SAMPLER_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT	= 0x194,
        SAMPLER_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR	= 0x195,
        SAMPLER_FILTER_MAXIMUM_ANISOTROPIC	= 0x1d5
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


struct SamplerDesc
{
    SamplerAddressMode _addressU;
    SamplerAddressMode _addressV;
    SamplerAddressMode _addressW;
    R32 _borderColor[4];
    ComparisonFunc _comparisonFunc;
    SamplerFilter _filter;    
    R32 _maxAnisotropy;
    R32 _minLod;
    R32 _maxLod;
    R32 _mipLodBias;
};


struct StaticSamplerDesc : public SamplerDesc
{
    U32 _registerSpace;
    U32 _shaderRegister;
};

enum DescriptorTableFlag
{
    DESCRIPTOR_TABLE_FLAG_APPEND = 0,
    DESCRIPTOR_TABLE_FLAG_RESET = (1<<0)
};

typedef U32 DescriptorTableFlags;

class DescriptorTable : public GPUObject
{
public:
    enum DescriptorTableType { DESCRIPTOR_TABLE_SRV_UAV_CBV, DESCRIPTOR_TABLE_SAMPLER };
    virtual ~DescriptorTable() { }
    virtual void setShaderResourceViews(ShaderResourceView** resources, U32 bufferCount) { }
    virtual void setUnorderedAccessViews(UnorderedAccessView** uavs, U32 uavCount) { }
    virtual void setConstantBuffers(Resource** buffer, U32 bufferCount) { }
    virtual void setSamplers(Sampler** samplers, U32 samplerCount) { }
    virtual void initialize(DescriptorTableType type, U32 totalCount) { }
    virtual void update(DescriptorTableFlags flags = DESCRIPTOR_TABLE_FLAG_APPEND) { }
};


enum PipelineLayoutType
{
    PIPELINE_LAYOUT_TYPE_DESCRIPTOR_TABLE,
    PIPELINE_LAYOUT_TYPE_CONSTANTS,
    PIPELINE_LAYOUT_TYPE_CBV,
    PIPELINE_LAYOUT_TYPE_SRV,
    PIPELINE_LAYOUT_TYPE_UAV,
    PIPELINE_LAYOUT_TYPE_SAMPLERS
};


struct PipelineLayout
{
  PipelineLayoutType _type;
  U32 _numConstantBuffers;
  U32 _numSamplers;
  U32 _numUnorderedAcessViews;
  U32 _numShaderResourceViews;
};

class RootSignature : public GPUObject
{
public:
    virtual void initialize(ShaderVisibilityFlags visibleFlags,
                            PipelineLayout* pLayouts, 
                            U32 numLayouts,
                            StaticSamplerDesc* pStaticSamplers = nullptr,
                            U32 staticSamplerCount = 0) { }
};


struct ShaderByteCode 
{
  void* _pByteCode;
  size_t _szBytes;
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


enum RayTracingHitGroupType
{
    RAYTRACING_HITGROUP_TYPE_PROCEDURAL_PRIMITIVE,
    RAYTRACING_HITGROUP_TYPE_TRIANGLES
};


struct AccelerationStructureInputs
{
};


struct AccelerationStructureGeometry : public AccelerationStructureInputs
{
    RayTracingHitGroupType _type;
    union {
        struct {
            Resource* _aabbResource;
            U32 _count;
            U32 strideInBytes;
        } aabbs;
        struct {
            Resource* _indexBuffer;
            Resource* _vertexBuffer;
            U32 strideInBytes;
            U32 _indexCount;
            DXGI_FORMAT _indexFormat;
            DXGI_FORMAT _vertexFormat;
            U32 _vertexCount;
            U32 _vertexStrideInBytes;
        } _tris;
    };
};


struct AccelerationStructureTopLevelInfo : public AccelerationStructureInputs
{
    U32 _instances;
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


enum RayTracingShader
{
    RAYTRACING_SHADER_RAYGEN = 0,
    RAYTRACING_SHADER_INTERSECTION = (RAYTRACING_SHADER_RAYGEN + 1),
    RAYTRACING_SHADER_MISS = (RAYTRACING_SHADER_INTERSECTION + 1),
    RAYTRACING_SHADER_CLOSESTHIT = (RAYTRACING_SHADER_MISS + 1),
    RAYTRACING_SHADER_ANYHIT = (RAYTRACING_SHADER_CLOSESTHIT + 1),
    RAYTRACING_SHADER_END = (RAYTRACING_SHADER_ANYHIT + 1)
};


struct RayTracingPipelineInfo
{
    RootSignature* pGlobalRootSignature;

    // Local Root signatures define which shaders can have unique arguments passed. 
    RootSignature* pLocalRootSignatures [RAYTRACING_SHADER_END];
    // Names of the entry points for the shader. Each name corresponds to the 
    // entry point of each shader type denoted by semantics in shader code.
    wchar_t* shaderNames                [RAYTRACING_SHADER_END];
    // Name of the hit group, which defines the group within aabb or geometry that is intersected by a ray. Hitgroup interesected will execute shaders associated with it.
    wchar_t* hitGroupName;
    ShaderByteCode fullShaderCode;
    RayTracingHitGroupType hitGroupType;
    // Size of the payload.
    UINT payloadSzBytes;
    // Size of the attribute.
    UINT attribSzBytes;
    UINT maxRecursionDepth;
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
    RootSignature* _pRootSignature;
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


typedef TargetView IndexBufferView;

class RenderPass : public GPUObject
{
public:
    virtual void setRenderTargets(RenderTargetView** pRenderTargets, U32 renderTargetCount) { }
    virtual void setDepthStencil(DepthStencilView* pDepthStencil) { }
};


class CommandList : public GPUObject
{
public:
    static const U64 kSwapchainRenderTargetId = 0xffffffffffffffff;

    CommandList() { }
    virtual ~CommandList() { }

    virtual void init() { }
    virtual void destroy() { }

    virtual void reset(const char* debugTag = nullptr) { }

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
    virtual void setComputeRootConstantBufferView(U32 rootParameterIndex, Resource* pConstantBuffer, U64 offset = 0ull) { }
    virtual void setComputeRootShaderResourceView(U32 rootParameterIndex, Resource* pShaderResourceView) { }
    virtual void setGraphicsRootConstantBufferView(U32 rootParameterIndex, Resource* pConstantBuffer, U64 offset = 0ull) { }
    virtual void setGraphicsRootShaderResourceView(U32 rootParameterIndex, Resource* pShaderResourceView) { }
    virtual void setGraphicsRoot32BitConstant(U32 rootParameterIndex) { }
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
    virtual void setMarker(const char* tag = nullptr) { }
    B32 isRecording() const { return _isRecording; }

protected:
    B32 _isRecording;
};

/*
    Behind every great renderer is a powerful Hardware Interface that handles the translation between
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
    virtual void createQueue(CommandQueue** ppQueue, CommandQueueType type) { }
    virtual void createRenderTargetView(RenderTargetView** rtv, Resource* texture, const RenderTargetViewDesc& desc) { }
    virtual void createUnorderedAccessView(UnorderedAccessView** uav, Resource* texture, const UnorderedAccessViewDesc& desc) { }
    virtual void createShaderResourceView(ShaderResourceView** srv, 
                                          Resource* resource, 
                                          const ShaderResourceViewDesc& desc) { }
    virtual void createDepthStencilView(DepthStencilView** dsv, Resource* texture, const DepthStencilViewDesc& desc) { }
    virtual void createGraphicsPipelineState(GraphicsPipeline** pipline,
                                             const GraphicsPipelineInfo* pInfo) { }
    virtual void createComputePipelineState(ComputePipeline** pipeline,
                                            const ComputePipelineInfo* pInfo) { }
    virtual void createRayTracingPipelineState(RayTracingPipeline** ppPipeline, 
                                               const RayTracingPipelineInfo* pInfo) { }

    virtual void createAccelerationStructure(Resource** ppResource,
                                             const AccelerationStructureGeometry* geometryInfos, 
                                             U32 geometryCount,
                                             const AccelerationStructureTopLevelInfo* pTopLevelInfo) { }
  
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

    virtual void createSampler(Sampler** sampler, const SamplerDesc* pDesc) { }
    virtual void destroySampler(Sampler* sampler) { }
    virtual void destroyDescriptorTable(DescriptorTable* table) { }

    virtual RendererT getSwapchainQueue() { return 0; }

    virtual void createCommandList(CommandList** pList) { }

    virtual RenderPass* getBackbufferRenderPass() { return nullptr; }
    virtual RenderTargetView* getSwapchainRenderTargetView() { return nullptr; }
    virtual void createFence(Fence** ppFence) { }
    virtual void destroyFence(Fence* pFence) { }

    B32 isHardwareRaytracingCompatible() const { return m_hardwareRaytracingCompatible; }
    B32 isHardwareMachineLearningCompatible() const { return m_harwareMachineLearningCompatible; }
    B32 isHardwareMeshShadingCompatible() const { return m_hardwareMeshShadingCompatible; }

protected:

    IDXGISwapChain1* m_pSwapChain;
    B32 m_hardwareRaytracingCompatible;
    B32 m_harwareMachineLearningCompatible;
    B32 m_hardwareMeshShadingCompatible;
};

} // gfx