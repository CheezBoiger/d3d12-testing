

#include "D3D12Backend.h"
#include "CommandListD3D12.h"
#include "D3D12MemAlloc.h"
#include "MemoryAllocatorD3D12.h"
#include "RootSignatureD3D12.h"
#include "PipelineStatesD3D12.h"
#include "DescriptorTableD3D12.h"
#include <string>

#include <pix.h>

namespace gfx
{

D3D12Backend* getBackendD3D12()
{
  static D3D12Backend backend;
  return &backend;
}

D3D12MA::Allocator* pAllocator = nullptr;
MemoryAllocatorD3D12* pCustomMemoryAllocator = nullptr;

D3D12_RESOURCE_DIMENSION getDimension(ResourceDimension dimension)
{
    switch (dimension) {
        case RESOURCE_DIMENSION_2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case RESOURCE_DIMENSION_3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}


D3D12_RESOURCE_STATES getNativeBindFlags(ResourceBindFlags binds,
                                         D3D12_HEAP_TYPE type)
{
  D3D12_RESOURCE_STATES flags = D3D12_RESOURCE_STATE_COMMON;
  if (type == D3D12_HEAP_TYPE_READBACK)
    return D3D12_RESOURCE_STATE_COPY_DEST;
  if (type == D3D12_HEAP_TYPE_UPLOAD) 
    return D3D12_RESOURCE_STATE_GENERIC_READ;

  if (binds & RESOURCE_BIND_RENDER_TARGET)
    flags = D3D12_RESOURCE_STATE_RENDER_TARGET;
  if (binds & RESOURCE_BIND_SHADER_RESOURCE)
    flags = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  if (binds & RESOURCE_BIND_UNORDERED_ACCESS)
    flags = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
  if (binds & RESOURCE_BIND_DEPTH_STENCIL)
    flags = D3D12_RESOURCE_STATE_DEPTH_WRITE;
  if (binds & RESOURCE_BIND_CONSTANT_BUFFER)
    flags = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  if (binds & RESOURCE_BIND_INDEX_BUFFER)
    flags = D3D12_RESOURCE_STATE_INDEX_BUFFER;
  if (flags & RESOURCE_BIND_VERTEX_BUFFER)
    flags = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  return flags;
}


D3D12_RESOURCE_FLAGS getNativeAllowFlags(ResourceBindFlags binds)
{
  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
  if (binds & RESOURCE_BIND_RENDER_TARGET)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  if (binds & RESOURCE_BIND_UNORDERED_ACCESS)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  if (binds & RESOURCE_BIND_DEPTH_STENCIL)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  return flags;
}


D3D12_PRIMITIVE_TOPOLOGY_TYPE getNativeTopologyType(PrimitiveTopology topology)
{
  switch (topology) {
    case PRIMITIVE_TOPOLOGY_POINTS: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case PRIMITIVE_TOPOLOGY_LINES: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case PRIMITIVE_TOPOLOGY_TRIANGLES: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case PRIMITIVE_TOPOLOGY_PATCHES: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    default: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
  }
}


D3D12_CULL_MODE getCullMode(CullMode cullMode)
{
  switch (cullMode) {
    case CULL_MODE_BACK: return D3D12_CULL_MODE_BACK;
    case CULL_MODE_NONE: return D3D12_CULL_MODE_NONE;
    case CULL_MODE_FRONT: 
    default: return D3D12_CULL_MODE_FRONT;
  }
}


D3D12_FILL_MODE getFillMode(FillMode fillMode)
{
  switch (fillMode) {
    case FILL_MODE_WIREFRAME: return D3D12_FILL_MODE_WIREFRAME;
    case FILL_MODE_SOLID:
    default: return D3D12_FILL_MODE_SOLID;
  }
}


D3D12_INDEX_BUFFER_STRIP_CUT_VALUE getIBCutValue(IBCutValue cutValue)
{
  switch (cutValue) {
    case IB_CUT_VALUE_CUT_0xFFFF: return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
    case IB_CUT_VALUE_CUT_0xFFFFFFFF: return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
    case IB_CUT_VALUE_DISABLED: 
    default: return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
  }
}


D3D12_STENCIL_OP getStencilOp(StencilOp op)
{
  switch (op) {
    case STENCIL_OP_DECR: return D3D12_STENCIL_OP_DECR;
    case STENCIL_OP_DECR_SAT: return D3D12_STENCIL_OP_DECR_SAT;
    case STENCIL_OP_INCR: return D3D12_STENCIL_OP_INCR;
    case STENCIL_OP_INCR_SAT: return D3D12_STENCIL_OP_INCR_SAT;
    case STENCIL_OP_INVERT: return D3D12_STENCIL_OP_INVERT;
    case STENCIL_OP_KEEP: return D3D12_STENCIL_OP_KEEP;
    case STENCIL_OP_REPLACE: return D3D12_STENCIL_OP_REPLACE;
    case STENCIL_OP_ZERO: 
    default: return D3D12_STENCIL_OP_ZERO; 
  }
}


D3D12_PRIMITIVE_TOPOLOGY getPrimitiveTopology(PrimitiveTopology topology)
{
  switch (topology) {
    case PRIMITIVE_TOPOLOGY_LINES: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case PRIMITIVE_TOPOLOGY_PATCHES: return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    case PRIMITIVE_TOPOLOGY_POINTS: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    case PRIMITIVE_TOPOLOGY_TRIANGLES: 
    default: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  }
}


D3D12_COMPARISON_FUNC getComparisonFunc(ComparisonFunc func)
{
  switch (func) {
    case COMPARISON_FUNC_EQUAL: return D3D12_COMPARISON_FUNC_EQUAL;
    case COMPARISON_FUNC_GREATER: return D3D12_COMPARISON_FUNC_GREATER;
    case COMPARISON_FUNC_GREATER_EQUAL: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case COMPARISON_FUNC_LESS: return D3D12_COMPARISON_FUNC_LESS;
    case COMPARISON_FUNC_LESS_EQUAL: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case COMPARISON_FUNC_NOT_EQUAL: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case COMPARISON_FUNC_ALWAYS: return D3D12_COMPARISON_FUNC_ALWAYS; 
    case COMPARISON_FUNC_NEVER:
    default: return D3D12_COMPARISON_FUNC_NEVER;
  }
}


D3D12_FILTER getNativeFilter(SamplerFilter filter)
{
    switch (filter) {
        case SAMPLER_FILTER_MIN_MAG_POINT_MIP_LINEAR: return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        case SAMPLER_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT: return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case SAMPLER_FILTER_MIN_POINT_MAG_MIP_LINEAR: return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        case SAMPLER_FILTER_MIN_LINEAR_MAG_MIP_POINT: return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        case SAMPLER_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR: return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case SAMPLER_FILTER_MIN_MAG_LINEAR_MIP_POINT: return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        case SAMPLER_FILTER_MIN_MAG_MIP_LINEAR: return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        case SAMPLER_FILTER_ANISOTROPIC: return D3D12_FILTER_ANISOTROPIC;
        case SAMPLER_FILTER_COMPARISON_MIN_MAG_MIP_POINT: return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        case SAMPLER_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR: return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        case SAMPLER_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT: return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case SAMPLER_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR: return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        case SAMPLER_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT: return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
        case SAMPLER_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR: return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case SAMPLER_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT: return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        case SAMPLER_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR: return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;   
        case SAMPLER_FILTER_COMPARISON_ANISOTROPIC: return D3D12_FILTER_COMPARISON_ANISOTROPIC;
        case SAMPLER_FILTER_MINIMUM_MIN_MAG_MIP_POINT: return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
        case SAMPLER_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR: return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
        case SAMPLER_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT: return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case SAMPLER_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR: return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
        case SAMPLER_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT: return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
        case SAMPLER_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR: return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case SAMPLER_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT: return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
        case SAMPLER_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR: return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
        case SAMPLER_FILTER_MINIMUM_ANISOTROPIC: return D3D12_FILTER_MINIMUM_ANISOTROPIC;
        case SAMPLER_FILTER_MAXIMUM_MIN_MAG_MIP_POINT: return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
        case SAMPLER_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR: return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
        case SAMPLER_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT: return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case SAMPLER_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR: return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
        case SAMPLER_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT: return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
        case SAMPLER_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR: return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case SAMPLER_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT: return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
        case SAMPLER_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR: return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
        case SAMPLER_FILTER_MAXIMUM_ANISOTROPIC: return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
        case SAMPLER_FILTER_MIN_MAG_MIP_POINT: 
        default: return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }   
}   


D3D12_TEXTURE_ADDRESS_MODE getNativeTextureAddress(SamplerAddressMode mode)
{
    switch (mode) {
        case SAMPLER_ADDRESS_MODE_BORDER: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case SAMPLER_ADDRESS_MODE_MIRROR: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case SAMPLER_ADDRESS_MODE_MIRROR_ONCE: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        case SAMPLER_ADDRESS_MODE_WRAP: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case SAMPLER_ADDRESS_MODE_CLAMP: 
        default: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    }
}


D3D12_DEPTH_WRITE_MASK getDepthWriteMask(DepthWriteMask mask)
{
  switch (mask) {
    case DEPTH_WRITE_MASK_ALL: return D3D12_DEPTH_WRITE_MASK_ALL;
    case DEPTH_WRITE_MASK_ZERO:
    default: return D3D12_DEPTH_WRITE_MASK_ZERO;
  }
}


D3D12_INPUT_CLASSIFICATION getD3D12InputClassification(InputClassification c)
{
  switch (c) {
    case INPUT_CLASSIFICATION_PER_INSTANCE: return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    case INPUT_CLASSIFICATION_PER_VERTEX:
    default: return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
  }
}


D3D12_HIT_GROUP_TYPE getHitGroupType(RayTracingHitGroupType type)
{
    switch (type) {
        case RAYTRACING_HITGROUP_TYPE_PROCEDURAL_PRIMITIVE: return D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE; 
        case RAYTRACING_HITGROUP_TYPE_TRIANGLES:
        default: return D3D12_HIT_GROUP_TYPE_TRIANGLES;
    }
}


void processRasterizationState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, 
                               const RasterizationStateInfo& rasterState)
{
  desc.RasterizerState.AntialiasedLineEnable = rasterState._antialiasedLinesEnable;
  desc.RasterizerState.ConservativeRaster = rasterState._conservativeRasterizationEnable 
                                          ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON 
                                          : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF; 
  desc.RasterizerState.CullMode = getCullMode(rasterState._cullMode);
  desc.RasterizerState.FillMode = getFillMode(rasterState._fillMode);
  desc.RasterizerState.DepthBias = rasterState._depthBias;
  desc.RasterizerState.MultisampleEnable = rasterState._multisampleEnable;
  desc.RasterizerState.DepthClipEnable = rasterState._depthClipEnable;
  desc.RasterizerState.DepthBiasClamp = rasterState._depthBiasClamp;
  desc.RasterizerState.ForcedSampleCount = rasterState._forcedSampleCount;
  desc.RasterizerState.FrontCounterClockwise = rasterState._frontCounterClockwise;
  desc.RasterizerState.SlopeScaledDepthBias = rasterState._slopedScaledDepthBias;
}


void processDepthStencilState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
                              const DepthStencilStateInfo& dsState)
{
  desc.DepthStencilState.BackFace.StencilDepthFailOp = getStencilOp(dsState._backFace._stencilDepthFailOp);
  desc.DepthStencilState.BackFace.StencilFailOp = getStencilOp(dsState._backFace._stencilFailOp);
  desc.DepthStencilState.BackFace.StencilPassOp = getStencilOp(dsState._backFace._stencilPassOp);
  desc.DepthStencilState.BackFace.StencilFunc = getComparisonFunc(dsState._backFace._stencilFunc);

  desc.DepthStencilState.FrontFace.StencilDepthFailOp = getStencilOp(dsState._frontFace._stencilDepthFailOp);
  desc.DepthStencilState.FrontFace.StencilFailOp = getStencilOp(dsState._frontFace._stencilFailOp);
  desc.DepthStencilState.FrontFace.StencilPassOp = getStencilOp(dsState._frontFace._stencilPassOp);
  desc.DepthStencilState.FrontFace.StencilFunc = getComparisonFunc(dsState._frontFace._stencilFunc);

  desc.DepthStencilState.DepthEnable = dsState._depthEnable;
  desc.DepthStencilState.DepthFunc = getComparisonFunc(dsState._depthFunc);
  desc.DepthStencilState.DepthWriteMask = getDepthWriteMask(dsState._depthWriteMask);
  
  desc.DepthStencilState.StencilEnable = dsState._stencilEnable;
  desc.DepthStencilState.StencilReadMask = dsState._stencilReadMask;
  desc.DepthStencilState.StencilWriteMask = dsState._stencilWriteMask;
}


D3D12_BLEND getBlend(Blend b)
{
  switch (b) {
    case BLEND_BLEND_FACTOR: return D3D12_BLEND_BLEND_FACTOR;
    case BLEND_DEST_ALPHA: return D3D12_BLEND_DEST_ALPHA;
    case BLEND_DEST_COLOR: return D3D12_BLEND_DEST_COLOR;
    case BLEND_INV_BLEND_FACTOR: return D3D12_BLEND_INV_BLEND_FACTOR;
    case BLEND_INV_DEST_ALPHA: return D3D12_BLEND_INV_DEST_ALPHA;
    case BLEND_INV_DEST_COLOR: return D3D12_BLEND_INV_DEST_COLOR;
    case BLEND_INV_SRC1_ALPHA: return D3D12_BLEND_INV_SRC1_ALPHA;
    case BLEND_INV_SRC1_COLOR: return D3D12_BLEND_INV_SRC1_COLOR;
    case BLEND_INV_SRC_COLOR: return D3D12_BLEND_INV_SRC_COLOR;
    case BLEND_ONE: return D3D12_BLEND_ONE;
    case BLEND_ZERO: return D3D12_BLEND_ZERO;
    default: return D3D12_BLEND_ZERO;
  }
}


D3D12_BLEND_OP getBlendOp(BlendOp op)
{
  switch (op) {
    case BLEND_OP_SUBTRACT: return D3D12_BLEND_OP_SUBTRACT;
    case BLEND_OP_REV_SUBTRACT: return D3D12_BLEND_OP_REV_SUBTRACT;
    case BLEND_OP_MIN: return D3D12_BLEND_OP_MIN;
    case BLEND_OP_MAX: return D3D12_BLEND_OP_MAX;
    case BLEND_OP_ADD: 
    default: return D3D12_BLEND_OP_ADD;
  }
}


D3D12_LOGIC_OP getLogicOp(LogicOp op)
{
  switch (op) {
    case LOGIC_OP_SET: return D3D12_LOGIC_OP_SET;
    case LOGIC_OP_COPY: return D3D12_LOGIC_OP_COPY;
    case LOGIC_OP_COPY_INVERTED: return D3D12_LOGIC_OP_COPY_INVERTED;
    case LOGIC_OP_NOOP: return D3D12_LOGIC_OP_NOOP;
    case LOGIC_OP_INVERT: return D3D12_LOGIC_OP_INVERT;
    case LOGIC_OP_AND: return D3D12_LOGIC_OP_AND;
    case LOGIC_OP_NAND: return D3D12_LOGIC_OP_NAND;
    case LOGIC_OP_OR: return D3D12_LOGIC_OP_OR;
    case LOGIC_OP_NOR: return D3D12_LOGIC_OP_NOR;
    case LOGIC_OP_XOR: return D3D12_LOGIC_OP_XOR;
    case LOGIC_OP_EQUIV: return D3D12_LOGIC_OP_EQUIV;
    case LOGIC_OP_AND_REVERSE: return D3D12_LOGIC_OP_AND_REVERSE;
    case LOGIC_OP_AND_INVERTED: return D3D12_LOGIC_OP_AND_INVERTED;
    case LOGIC_OP_OR_REVERSE: return D3D12_LOGIC_OP_OR_REVERSE;
    case LOGIC_OP_OR_INVERTED: return D3D12_LOGIC_OP_OR_INVERTED;
    case LOGIC_OP_CLEAR:
    default: return D3D12_LOGIC_OP_CLEAR;
  }
}


void processBlendState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
                       const BlendStateInfo& blendState)
{
  desc.BlendState.AlphaToCoverageEnable = blendState._alphaToCoverageEnable;
  desc.BlendState.IndependentBlendEnable = blendState._independentBlendEnable;
  
  for (U32 i = 0; i < 8; ++i) {
      desc.BlendState.RenderTarget[i].BlendEnable = blendState._renderTargets[i]._blendEnable;
      desc.BlendState.RenderTarget[i].BlendOp = getBlendOp(blendState._renderTargets[i]._blendOp);
      desc.BlendState.RenderTarget[i].BlendOpAlpha = getBlendOp(blendState._renderTargets[i]._blendOpAlpha);
      desc.BlendState.RenderTarget[i].DestBlend = getBlend(blendState._renderTargets[i]._dstBlend);
      desc.BlendState.RenderTarget[i].DestBlendAlpha = getBlend(blendState._renderTargets[i]._dstBlendAlpha);
      desc.BlendState.RenderTarget[i].LogicOp = getLogicOp(blendState._renderTargets[i]._logicOp);
      desc.BlendState.RenderTarget[i].LogicOpEnable = blendState._renderTargets[i]._logicOpEnable;
      desc.BlendState.RenderTarget[i].RenderTargetWriteMask = blendState._renderTargets[i]._renderTargetWriteMask;
      desc.BlendState.RenderTarget[i].SrcBlend = getBlend(blendState._renderTargets[i]._srcBlend);
      desc.BlendState.RenderTarget[i].SrcBlendAlpha = getBlend(blendState._renderTargets[i]._srcBlendAlpha);
  }
}


D3D12_RENDER_TARGET_VIEW_DESC processRenderTargetViewDesc(const RenderTargetViewDesc& desc)
{
    D3D12_RENDER_TARGET_VIEW_DESC rtv;
    rtv.Format = desc._format;
    switch (desc._dimension) {
        case RESOURCE_DIMENSION_1D: 
            {
                rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                rtv.Texture1D.MipSlice = desc._texture1D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_BUFFER:
            {
                rtv.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
                rtv.Buffer.FirstElement = desc._buffer._firstElement;
                rtv.Buffer.NumElements = desc._buffer._numElements;
            } break;
        case RESOURCE_DIMENSION_1D_ARRAY:
            {
                rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                rtv.Texture1DArray.ArraySize = desc._texture1DArray._arraySize;
                rtv.Texture1DArray.FirstArraySlice = desc._texture1DArray._firstArraySlice;
                rtv.Texture1DArray.MipSlice = desc._texture1DArray._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D:
            {
                rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtv.Texture2D.MipSlice = desc._texture2D._mipSlice;
                rtv.Texture2D.PlaneSlice = desc._texture2D._planeSlice;
            } break;
        case RESOURCE_DIMENSION_2D_ARRAY:
            {
                rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                rtv.Texture2DArray.ArraySize = desc._texture2DArray._arraySize;
                rtv.Texture2DArray.FirstArraySlice = desc._texture2DArray._firstArraySlice;
                rtv.Texture2DArray.MipSlice = desc._texture2DArray._mipSlice;
                rtv.Texture2DArray.PlaneSlice = desc._texture2DArray._planeSlice;
            } break;
        case RESOURCE_DIMENSION_3D:
            {
                rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                rtv.Texture3D.FirstWSlice = desc._texture3D._firstWSlice;
                rtv.Texture3D.MipSlice = desc._texture3D._mipSlice;
                rtv.Texture3D.WSize = desc._texture3D._wSize;
            } break;
        default: break;
    }
    return rtv;
}

D3D12_DSV_FLAGS processDepthStencilFlags(U32 flags)
{
    D3D12_DSV_FLAGS nativeFlags = D3D12_DSV_FLAG_NONE;
    if (flags & DEPTH_STENCIL_FLAG_ONLY_DEPTH) nativeFlags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
    if (flags & DEPTH_STENCIL_FLAG_ONLY_STENCIL) nativeFlags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
    return nativeFlags;
}


D3D12_DEPTH_STENCIL_VIEW_DESC processDepthStencilViewDesc(const DepthStencilViewDesc& desc)
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsv;
    dsv.Format = desc._format;
    dsv.Flags = processDepthStencilFlags(desc._flags);
    switch (desc._dimension) {
        case RESOURCE_DIMENSION_1D:
            {
                dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                dsv.Texture1D.MipSlice = desc._texture1D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_1D_ARRAY:
            {
                dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                dsv.Texture1DArray.ArraySize = desc._texture1DArray._arraySize;
                dsv.Texture1DArray.FirstArraySlice = desc._texture1DArray._firstArraySlice;
                dsv.Texture1DArray.MipSlice = desc._texture1DArray._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D:
            {
                dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                dsv.Texture2D.MipSlice = desc._texture2D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D_ARRAY:
            {
                dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                dsv.Texture2DArray.ArraySize = desc._texture2DArray._arraySize;
                dsv.Texture2DArray.FirstArraySlice = desc._texture2DArray._firstArraySlice;
                dsv.Texture2DArray.MipSlice = desc._texture2DArray._mipSlice;
            } break;
        default: break;
    }
    return dsv;
}

D3D12_BUFFER_SRV_FLAGS processSRVFlags(U32 flags)
{
    D3D12_BUFFER_SRV_FLAGS nativeFlags = D3D12_BUFFER_SRV_FLAG_NONE;
    if (flags & BUFFER_SRV_FLAGS_RAW) nativeFlags |= D3D12_BUFFER_SRV_FLAG_RAW;
    return nativeFlags;
}

D3D12_BUFFER_UAV_FLAGS processUAVFlags(U32 flags)
{
    D3D12_BUFFER_UAV_FLAGS nativeFlags = D3D12_BUFFER_UAV_FLAG_NONE;
    if (flags & BUFFER_UAV_FLAG_RAW) nativeFlags |= D3D12_BUFFER_UAV_FLAG_RAW;
    return nativeFlags;
}

D3D12_SHADER_RESOURCE_VIEW_DESC processShaderResourceViewDesc(const ShaderResourceViewDesc& desc)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srv;
    srv.Format = desc._format;
    switch (desc._dimension) {
        case RESOURCE_DIMENSION_BUFFER:
            {
                srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                srv.Buffer.FirstElement = desc._buffer._firstElement;
                srv.Buffer.Flags = processSRVFlags(desc._buffer._flags);
                srv.Buffer.NumElements = desc._buffer._numElements;
                srv.Buffer.StructureByteStride = desc._buffer._structureByteStride;
            } break;
        case RESOURCE_DIMENSION_1D:
            {
                srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                srv.Texture1D.MipLevels = desc._texture1D._mipLevels;
                srv.Texture1D.MostDetailedMip = desc._texture1D._mostDetailedMip;
                srv.Texture1D.ResourceMinLODClamp = desc._texture1D._resourceMinLODClamp;
            } break;
        case RESOURCE_DIMENSION_1D_ARRAY:
            {
                srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srv.Texture1DArray.ArraySize = desc._texture1DArray._arraySize;
                srv.Texture1DArray.FirstArraySlice = desc._texture1DArray._firstArraySlice;
                srv.Texture1DArray.MipLevels = desc._texture1DArray._mipLevels;
                srv.Texture1DArray.MostDetailedMip = desc._texture1DArray._mostDetailedMip;
                srv.Texture1DArray.ResourceMinLODClamp = desc._texture1DArray._resourceMinLODClamp;
            } break;
        case RESOURCE_DIMENSION_2D:
            {
                srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srv.Texture2D.MipLevels = desc._texture2D._mipLevels;
                srv.Texture2D.MostDetailedMip = desc._texture2D._mostDetailedMip;
                srv.Texture2D.PlaneSlice = desc._texture2D._planeSlice;
                srv.Texture2D.ResourceMinLODClamp = desc._texture2D._resourceMinLODClamp;
            } break;
        case RESOURCE_DIMENSION_2D_ARRAY:
            {
                srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                srv.Texture2DArray.ArraySize = desc._texture2DArray._arraySize;
                srv.Texture2DArray.FirstArraySlice = desc._texture2DArray._firstArraySlice;
                srv.Texture2DArray.MipLevels = desc._texture2DArray._mipLevels;
                srv.Texture2DArray.MostDetailedMip = desc._texture2DArray._mostDetailedMip;
                srv.Texture2DArray.PlaneSlice = desc._texture2DArray._planeSlice;
                srv.Texture2DArray.ResourceMinLODClamp = desc._texture2DArray._resourceMinLodClamp;
            } break;
        case RESOURCE_DIMENSION_3D:
            {
                srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                srv.Texture3D.MipLevels = desc._texture3D._mipLevels;
                srv.Texture3D.MostDetailedMip = desc._texture3D._mostDetailedMip;
                srv.Texture3D.ResourceMinLODClamp = desc._texture3D._resourceMinLODClamp;
            } break;
        case RESOURCE_DIMENSION_TEXTURE_CUBE:
            {
                srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                srv.TextureCube.MipLevels = desc._textureCube._mipLevels;
                srv.TextureCube.MostDetailedMip = desc._textureCube._mostDetailedMip;
                srv.TextureCube.ResourceMinLODClamp = desc._textureCube._resourceMinLODClamp;
            } break;
        case RESOURCE_DIMENSION_TEXTURE_CUBE_ARRAY:
            {
                srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                srv.TextureCubeArray.First2DArrayFace = desc._textureCubeArray._first2DArrayFace;
                srv.TextureCubeArray.MipLevels = desc._textureCubeArray._mipLevels;
                srv.TextureCubeArray.MostDetailedMip = desc._textureCubeArray._mostDetailedMip;
                srv.TextureCubeArray.NumCubes = desc._textureCubeArray._numCubes;
                srv.TextureCubeArray.ResourceMinLODClamp = desc._textureCubeArray._resourceMinLODClamp;
            } break;
        case RESOURCE_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE:
            {
                // TODO: Need to find a better way to handle this. Essentially we will need gpu address to pass for ray tracing pipelines.
                srv.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
                srv.RaytracingAccelerationStructure.Location = 
                    static_cast<BufferD3D12*>(desc._rayTracingAccelerationStructure._location)->pBackend->getResource(
                        desc._rayTracingAccelerationStructure._location->getUUID())->GetGPUVirtualAddress();
            } break;
        default: break;
    }
    return srv;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC processUnorderedAccessViewDesc(const UnorderedAccessViewDesc& desc)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uav;
    uav.Format = desc._format;
    switch (desc._dimension) {
        case RESOURCE_DIMENSION_BUFFER:
            {
                uav.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                uav.Buffer.CounterOffsetInBytes = desc._buffer._counterOffsetInBytes;
                uav.Buffer.FirstElement = desc._buffer._firstElement;
                uav.Buffer.Flags = processUAVFlags(desc._buffer._flags);
                uav.Buffer.NumElements = desc._buffer._numElements;
                uav.Buffer.StructureByteStride = desc._buffer._structureByteStride;
            } break;
        case RESOURCE_DIMENSION_1D:
            {
                uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                uav.Texture1D.MipSlice = desc._texture1D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_1D_ARRAY:
            {
                uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                uav.Texture1DArray.ArraySize = desc._texture1DArray._arraySize;
                uav.Texture1DArray.FirstArraySlice = desc._texture1DArray._firstArraySlice;
                uav.Texture1DArray.MipSlice = desc._texture1DArray._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D:
            {
                uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uav.Texture2D.MipSlice = desc._texture2D._mipSlice;
                uav.Texture2D.PlaneSlice = desc._texture2D._planeSlice;
            } break;
        case RESOURCE_DIMENSION_2D_ARRAY:
            {
                uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                uav.Texture2DArray.ArraySize = desc._texture2DArray._arraySize;
                uav.Texture2DArray.FirstArraySlice = desc._texture2DArray._firstArraySlice;
                uav.Texture2DArray.MipSlice = desc._texture2DArray._mipSlice;
                uav.Texture2DArray.PlaneSlice = desc._texture2DArray._planeSlice;
            } break;
        case RESOURCE_DIMENSION_3D:
            {
                uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                uav.Texture3D.FirstWSlice = desc._texture3D._firstWSlice;
                uav.Texture3D.MipSlice = desc._texture3D._mipSlice;
                uav.Texture3D.WSize = desc._texture3D._wSize;
            } break;
        default: break;
    }
    return uav;
}


void* BufferD3D12::map(U64 start, U64 sz)
{
  void* pData =  nullptr;
  D3D12_RANGE range = { };
  range.Begin = start;
  range.End = start + sz;
  ID3D12Resource* pResource = pBackend->getResource(getUUID());
  HRESULT result = pResource->Map(0, &range, &pData); 
  DX12ASSERT(result);
  return pData;
}


void BufferD3D12::unmap(U64 start, U64 sz)
{
  D3D12_RANGE range;
  range.Begin = start;
  range.End = start + sz;
  ID3D12Resource* pResource = pBackend->getResource(getUUID());
  pResource->Unmap(0, &range);
}


D3D12Backend::D3D12Backend()
{

}


IDXGIFactory4* D3D12Backend::createFactory()
{
    IDXGIFactory4* pFactory = nullptr;
    HRESULT result = CreateDXGIFactory2(0, __uuidof(IDXGIFactory4), (void**)& pFactory);
    if (FAILED(result)) {
        DEBUG("Failed to create dxgi factory.");
        return nullptr;
    }
    return pFactory;
}


void D3D12Backend::initialize(HWND handle, 
                              bool isFullScreen, 
                              const GpuConfiguration& configs)
{
    if (!handle) return;
    IDXGIFactory4* factory = createFactory();

#if _DEBUG
    D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&debug0);
    debug0->EnableDebugLayer();
    debug0->QueryInterface<ID3D12Debug1>(&debug1);
    debug1->SetEnableGPUBasedValidation(true);
#endif
    queryForDevice(factory);
    createCommandAllocators();
    createGraphicsQueue();
    createSwapChain(factory,
                    handle, 
                    configs._renderWidth,
                    configs._renderHeight,
                    configs._desiredBuffers,
                    configs._windowed);
    createHeaps();
    createDescriptorHeaps();
    querySwapChain();
    

    factory->Release();
}


void D3D12Backend::queryForDevice(IDXGIFactory4* pFactory)
{

    HRESULT result = 0;
    IDXGIAdapter1* pDesiredAdapter = nullptr;
    IDXGIAdapter1* pTempAdapter = nullptr;
    for (uint32_t i = 0; pFactory->EnumAdapters1(i, &pTempAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc = { };
        pTempAdapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }
        pDesiredAdapter = pTempAdapter;
        break;;
    }

    result = D3D12CreateDevice(pDesiredAdapter, 
                               D3D_FEATURE_LEVEL_12_0, 
                               __uuidof(ID3D12Device), 
                                (void**)& m_pDevice);
    DX12ASSERT(result);
    if (FAILED(result)) {
        DEBUG("Failed to create d3d12 device!");
        return;
    }

    // Check if supports raytracing.
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureOptions;
    DX12ASSERT(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, 
                                              &featureOptions, 
                                              sizeof(featureOptions)));
    m_hardwareRaytracingCompatible = (featureOptions.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);

    D3D12MA::ALLOCATOR_DESC allocDesc = { };
    allocDesc.pDevice = m_pDevice;
    allocDesc.PreferredBlockSize = 1 * GB_1;
    allocDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
    D3D12MA::CreateAllocator(&allocDesc, &pAllocator);
    pCustomMemoryAllocator = new MemoryAllocatorD3D12();
    pCustomMemoryAllocator->initialize(m_pDevice);
}


void D3D12Backend::createSwapChain(IDXGIFactory4* pFactory,
                                   HWND handle, 
                                   U32 renderWidth, 
                                   U32 renderHeight, 
                                   U32 desiredBuffers, 
                                   B32 windowed)
{
    m_swapchainDesc = { };
    m_swapchainDesc.BufferCount = desiredBuffers;
    m_swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    m_swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_swapchainDesc.Width = renderWidth;
    m_swapchainDesc.Height = renderHeight;
    m_swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    m_swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    m_swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    m_swapchainDesc.SampleDesc.Count = 1;
    m_swapchainDesc.SampleDesc.Quality = 0;
    
    HRESULT result = pFactory->CreateSwapChainForHwnd(m_pCommandQueues[kGraphicsQueueId], 
                                                      handle, 
                                                      &m_swapchainDesc, 
                                                      nullptr, 
                                                      nullptr, 
                                                      &m_pSwapChain);
    DX12ASSERT(result);
    if (FAILED(result)) {
        DEBUG("Failed to create swapchain!");
        return;
    }

    DX12ASSERT(m_pSwapChain->QueryInterface<IDXGISwapChain3>(&m_pD3D12Swapchain));
}


void D3D12Backend::querySwapChain()
{
    m_frameResources.resize(m_swapchainDesc.BufferCount);
    DXGI_SWAP_CHAIN_DESC swapchainDesc = { };
    m_pSwapChain->GetDesc(&swapchainDesc);

    ID3D12DescriptorHeap* pRtvHeap = getDescriptorHeap(DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS);
    U32 incSz = m_pDevice->GetDescriptorHandleIncrementSize(pRtvHeap->GetDesc().Type);
    D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS];

    for (U32 i = 0; i < m_frameResources.size(); ++i) {
        FrameResource& resource = m_frameResources[i];
        ID3D12Resource* pResource = nullptr;
        HRESULT result = m_pSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&pResource);
        m_resources[FrameResource::kFrameResourceId].push_back(pResource);
        m_frameResources[i]._rtv._currentState = D3D12_RESOURCE_STATE_COMMON;
        D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();
        if (FAILED(result)) {
            DEBUG("Failed to query from swapchain buffer!");
            continue;
        }
        resource._swapImage = pResource;
        
        DX12ASSERT(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                          __uuidof(ID3D12CommandAllocator), 
                                          (void**)&resource._pAllocator));
        DX12ASSERT(m_pDevice->CreateCommandList(0, 
                                                 D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 resource._pAllocator, 
                                                 nullptr, 
                                                 __uuidof(ID3D12GraphicsCommandList), 
                                                 (void**)&resource._cmdList));
        resource._cmdList->Close();

        D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { };
        renderTargetViewDesc.Format = swapchainDesc.BufferDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;
        renderTargetViewDesc.Texture2D.PlaneSlice = 0;
        m_pDevice->CreateRenderTargetView(pResource, &renderTargetViewDesc, rtvHandle);
        m_viewHandles[m_frameResources[i]._rtv.getUUID()].push_back(rtvHandle);

        rtvHandle.ptr += incSz;
        m_frameResources[i]._swapImage->SetName(TEXT("_swapchainBuffer"));
        m_frameResources[i]._fenceValue = 0;
        m_frameResources[i]._rtv._buffer = FrameResource::kFrameResourceId;
    }

    {
      m_pSwapchainPass = new RenderPassD3D12();
      m_pSwapchainPass->_renderTargetViews.resize(m_frameResources.size());
      for (U32 i = 0; i < m_frameResources.size(); ++i) {
        m_pSwapchainPass->_renderTargetViews[i] = &m_frameResources[i]._rtv;
      }
    }

    DX12ASSERT(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pPresentFence));
    m_pPresentEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}


void D3D12Backend::createGraphicsQueue()
{
  D3D12_COMMAND_QUEUE_DESC desc = { };
  desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  desc.Priority = 0;
  desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  desc.NodeMask = 0;
  ID3D12CommandQueue* pQueue = nullptr;
  DX12ASSERT(m_pDevice->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), (void**)&pQueue));
  m_pCommandQueues[kGraphicsQueueId] = pQueue;
}


void D3D12Backend::createBuffer(Resource** buffer, 
                                ResourceUsage usage,
                                ResourceBindFlags binds, 
                                U32 widthBytes, 
                                U32 structureByteStride,
                                const TCHAR* debugName)
{
    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = DXGI_FORMAT_UNKNOWN;
    ID3D12Resource* pResource = nullptr;
    D3D12_RESOURCE_DESC desc = { };
    desc.Alignment = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = widthBytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Flags = getNativeAllowFlags(binds);
    D3D12MA::Allocation* alloc;
    D3D12MA::ALLOCATION_DESC allocDesc = { };
    allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;
    if (usage == RESOURCE_USAGE_GPU_TO_CPU)
      allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
    else if (usage == RESOURCE_USAGE_CPU_TO_GPU)
      allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
    else if (usage == RESOURCE_USAGE_DEFAULT)
      allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_STATES state = getNativeBindFlags(binds, allocDesc.HeapType);
    D3D12_CLEAR_VALUE* pClearValue = &clearValue;
    if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
      pClearValue = NULL;
    } else {
      D3D12_RESOURCE_ALLOCATION_INFO rAllocInfo = m_pDevice->GetResourceAllocationInfo(0, 1, &desc);
      (void)rAllocInfo;
    }
    HRESULT result = pAllocator->CreateResource(&allocDesc, 
                                                 &desc, 
                                                 state, 
                                                 pClearValue, 
                                                 &alloc, 
                                                 __uuidof(ID3D12Resource), 
                                                 (void**)&pResource); 
    //desc.Width = KB_1 * 128ull;
    //pCustomMemoryAllocator->allocate(m_pDevice, allocDesc.HeapType, state, pClearValue, desc);
    DX12ASSERT(result);
    if (debugName) {
      pResource->SetName(debugName);
    }
    BufferD3D12* pNativeBuffer = new BufferD3D12(this,
                                                 RESOURCE_DIMENSION_BUFFER,
                                                 usage,
                                                 binds,
                                                 structureByteStride);
    pNativeBuffer->pAllocation = alloc;
    pNativeBuffer->_currentResourceState = state;
    *buffer = pNativeBuffer; 
    m_resources[(*buffer)->getUUID()].push_back(pResource);
}


void D3D12Backend::createTexture(Resource** texture,
                                 ResourceDimension dimension,
                                 ResourceUsage usage,
                                 ResourceBindFlags binds,
                                 DXGI_FORMAT format,
                                 U32 width,
                                 U32 height,
                                 U32 depth,
                                 U32 structureByteStride,
                                 const TCHAR* debugName)
{
  D3D12_CLEAR_VALUE clearValue;
  clearValue.Format = format;

  ID3D12Resource* pResource = nullptr;
  D3D12_RESOURCE_DESC desc = {};
  desc.Alignment = 0;
  desc.Dimension = getDimension(dimension);
  desc.Width = width;
  desc.Height = height;
  desc.DepthOrArraySize = depth;
  desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  desc.Format = format;
  desc.MipLevels = 1;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Flags = getNativeAllowFlags(binds);

  D3D12MA::Allocation* alloc;
  D3D12MA::ALLOCATION_DESC allocDesc = {};
  allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;
  if (usage == RESOURCE_USAGE_GPU_TO_CPU)
    allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
  else if (usage == RESOURCE_USAGE_CPU_TO_GPU)
    allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
  else if (usage == RESOURCE_USAGE_DEFAULT)
    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

  D3D12_RESOURCE_STATES state = getNativeBindFlags(binds, allocDesc.HeapType);

  D3D12_CLEAR_VALUE* pClearValue = &clearValue;
  if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER || !(state & RESOURCE_BIND_RENDER_TARGET)) {
    pClearValue = NULL;
  } else {
    D3D12_RESOURCE_ALLOCATION_INFO rAllocInfo =
        m_pDevice->GetResourceAllocationInfo(0, 1, &desc);
    (void)rAllocInfo;
  }

  if (binds & RESOURCE_BIND_RENDER_TARGET) {
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 0.0f;
  } else if (binds & RESOURCE_BIND_DEPTH_STENCIL) {
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;
  }

  HRESULT result =
      pAllocator->CreateResource(&allocDesc, 
                                 &desc, 
                                 state, 
                                 pClearValue, 
                                 &alloc,
                                 __uuidof(ID3D12Resource), 
                                (void**)&pResource);
  DX12ASSERT(result);

  if (debugName) {
    pResource->SetName(debugName);
  }

  BufferD3D12* pNativeBuffer = new BufferD3D12(this, dimension, usage, binds, structureByteStride);
  pNativeBuffer->pAllocation = alloc;
  pNativeBuffer->_currentResourceState = state;

  *texture = pNativeBuffer;

  m_resources[(*texture)->getUUID()].push_back(pResource);
}


void D3D12Backend::destroyResource(Resource* buffer)
{
  for (size_t i = 0; i < m_resources[buffer->getUUID()].size(); ++i) {
    m_resources[buffer->getUUID()][i]->Release();
    m_resources[buffer->getUUID()][i] = nullptr;
  }
  delete buffer;
}


void D3D12Backend::createRenderPass(RenderPass** pass,
                                    U32 rtvSize,
                                    B32 hasDepthStencil)
{
  *pass = new RenderPassD3D12();
  
}


void D3D12Backend::destroyRenderPass(RenderPass* pass)
{

}


void D3D12Backend::createRenderTargetView(RenderTargetView** rtv, Resource* buffer, const RenderTargetViewDesc& desc)
{
  ViewHandleD3D12* pView = new ViewHandleD3D12();
  ID3D12DescriptorHeap* rtvHeap = getDescriptorHeap(DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS);
  D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS];

  U32 incSz = m_pDevice->GetDescriptorHandleIncrementSize(rtvHeap->GetDesc().Type);

  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = processRenderTargetViewDesc(desc);
  
  m_viewHandles[pView->getUUID()].resize(m_resources[buffer->getUUID()].size());

  for (size_t i = 0; i < m_resources[buffer->getUUID()].size(); ++i) {
    ID3D12Resource* pResource = getResource(buffer->getUUID(), i);
    D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();
    m_pDevice->CreateRenderTargetView(pResource, &rtvDesc, cpuHandle);
    m_viewHandles[pView->getUUID()][i] = cpuHandle;
    cpuHandle.ptr += incSz;
  }

  // Set to current state, in order to transition.
  pView->_currentState = static_cast<BufferD3D12*>(buffer)->_currentResourceState;
  pView->_buffer = buffer->getUUID();

  *rtv = pView;
}


void D3D12Backend::createUnorderedAccessView(UnorderedAccessView** uav, Resource* buffer, const UnorderedAccessViewDesc& desc)
{

}


void D3D12Backend::createShaderResourceView(ShaderResourceView** srv,
                                            Resource* buffer,
                                            const ShaderResourceViewDesc& desc) 
{
    ViewHandleD3D12* pView = new ViewHandleD3D12();
    ID3D12DescriptorHeap* srvHeap = getDescriptorHeap(DESCRIPTOR_HEAP_SRV_UAV_CBV);
    D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_SRV_UAV_CBV];
    U32 incSz = m_pDevice->GetDescriptorHandleIncrementSize(srvHeap->GetDesc().Type);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = processShaderResourceViewDesc(desc);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    
    m_viewHandles[pView->getUUID()].resize(m_resources[buffer->getUUID()].size());
    for (size_t i = 0; i < m_resources[buffer->getUUID()].size(); ++i) {
        ID3D12Resource* pResource = getResource(buffer->getUUID(), i);
        D3D12_RESOURCE_DESC desc = pResource->GetDesc();
        m_pDevice->CreateShaderResourceView(pResource, &srvDesc, cpuHandle);
        m_viewHandles[pView->getUUID()][i] = cpuHandle;
        cpuHandle.ptr += incSz;
    }
    
    pView->_currentState = D3D12_RESOURCE_STATE_COMMON;
    pView->_buffer = buffer->getUUID();
    
    *srv = pView;
}


void D3D12Backend::createDepthStencilView(DepthStencilView** dsv, Resource* buffer, const DepthStencilViewDesc& desc)
{
  ViewHandleD3D12* pView = new ViewHandleD3D12();
  *dsv = pView;
  pView->_buffer = buffer->getUUID();

  ID3D12DescriptorHeap* rtvHeap = getDescriptorHeap(DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS);
  D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS];

  U32 incSz = m_pDevice->GetDescriptorHandleIncrementSize(rtvHeap->GetDesc().Type);

  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = processDepthStencilViewDesc(desc);
  
  m_viewHandles[pView->getUUID()].resize(m_resources[buffer->getUUID()].size());
  for (size_t i = 0; i < m_resources[buffer->getUUID()].size(); ++i) {
    ID3D12Resource* pResource = getResource(buffer->getUUID(), i);
    D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();
    m_pDevice->CreateDepthStencilView(pResource, &dsvDesc, cpuHandle);
    m_viewHandles[pView->getUUID()][i] = cpuHandle;
    cpuHandle.ptr += incSz;
  }
}


void D3D12Backend::present()
{
#if 1
  if (m_frameResources[m_frameIndex]._rtv._currentState != D3D12_RESOURCE_STATE_PRESENT) {
    D3D12_RESOURCE_BARRIER barrier = { };
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_frameResources[m_frameIndex]._swapImage;
    barrier.Transition.StateBefore = m_frameResources[m_frameIndex]._rtv._currentState;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_frameResources[m_frameIndex]._cmdList->Reset(m_frameResources[m_frameIndex]._pAllocator, nullptr);
    m_frameResources[m_frameIndex]._cmdList->ResourceBarrier(1, &barrier);
    HRESULT hr = m_frameResources[m_frameIndex]._cmdList->Close(); 
    DX12ASSERT(hr);
    
    ID3D12CommandList* cmd[] = { m_frameResources[m_frameIndex]._cmdList };

    m_pCommandQueues[kGraphicsQueueId]->ExecuteCommandLists(1, cmd);
    m_frameResources[m_frameIndex]._rtv._currentState = D3D12_RESOURCE_STATE_COMMON;
    
  }
#endif
  HRESULT result = m_pD3D12Swapchain->Present(1, 0); 
  DX12ASSERT(result);

  U32 currFenceValue = m_frameResources[m_frameIndex]._fenceValue;
  m_pCommandQueues[kGraphicsQueueId]->Signal(m_pPresentFence, currFenceValue);
  m_frameIndex = m_pD3D12Swapchain->GetCurrentBackBufferIndex();
  if (m_pPresentFence->GetCompletedValue() < m_frameResources[m_frameIndex]._fenceValue) {
    m_pPresentFence->SetEventOnCompletion(m_frameResources[m_frameIndex]._fenceValue, m_pPresentEvent);
    WaitForSingleObjectEx(m_pPresentEvent, INFINITE, FALSE);
  }

  m_frameResources[m_frameIndex]._fenceValue = currFenceValue + 1;

  DX12ASSERT(m_frameResources[m_frameIndex]._pAllocator->Reset());
}


void D3D12Backend::createHeaps()
{
  for (U32 i = 0; i < DESCRIPTOR_HEAP_END; ++i) {

    D3D12_HEAP_DESC heapDesc = { };
    heapDesc.Properties.VisibleNodeMask = 0;
    heapDesc.Alignment = 0;
    heapDesc.Properties.CreationNodeMask = 0;
    heapDesc.SizeInBytes = 512 * MB_1; 
    switch (i) {
      case DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS:
      case DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS:
        // My device seems to break when CPUPageProperty and/or MemoryPool Preference are defined,
        // which doesn't seem to make sense why they exist when heap type already defines the mem type.
        heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES; 
        heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        break;
      default:
        heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
        heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    }

    ID3D12Heap* pHeap = nullptr;
    HRESULT result = m_pDevice->CreateHeap(&heapDesc, __uuidof(ID3D12Heap), (void**)&pHeap); 
    DX12ASSERT(result);

    m_pHeaps[i] = pHeap;
  }
}


void D3D12Backend::createDescriptorHeaps()
{
  for (U32 i = DESCRIPTOR_HEAP_START; i < DESCRIPTOR_HEAP_END; ++i) {
    D3D12_DESCRIPTOR_HEAP_DESC desc = { };
    desc.NodeMask = 0;
    desc.NumDescriptors = 2048;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    switch (i) {
      case DESCRIPTOR_HEAP_RENDER_TARGET_VIEWS:
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        break;
      case DESCRIPTOR_HEAP_DEPTH_STENCIL_VIEWS:
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        break;
      case DESCRIPTOR_HEAP_SAMPLER:
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        break;
      default:
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        break;
    }

    ID3D12DescriptorHeap* pDescHeap = nullptr;
    DX12ASSERT(m_pDevice->CreateDescriptorHeap(&desc, 
                                               __uuidof(ID3D12DescriptorHeap), 
                                               (void**)&pDescHeap));
    m_pDescriptorHeaps[i] = pDescHeap;
    m_descriptorHeapCurrentOffset[i] = pDescHeap->GetCPUDescriptorHandleForHeapStart();
  }
}


void D3D12Backend::createCommandList(CommandList** pList) 
{
  std::vector<ID3D12CommandAllocator*> allocs(m_frameResources.size());
  for (U32 i = 0; i < m_frameResources.size(); ++i)
    allocs[i] = m_frameResources[i]._pAllocator;

  CommandList* pNativeList = nullptr;
  pNativeList = new GraphicsCommandListD3D12(D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                            allocs.data(), 
                                            static_cast<U32>(allocs.size()));

  *pList = pNativeList;
  
}


void D3D12Backend::destroyCommandList(CommandList* pList)
{
  pList->destroy();
  delete pList;
}


void D3D12Backend::submit(RendererT queue, CommandList** cmdLists, U32 numCmdLists)
{
  ID3D12CommandQueue* pQueue = m_pCommandQueues[queue];
  static ID3D12CommandList* pNativeLists[64];

  for (U32 i = 0; i < numCmdLists; ++i) {
    GraphicsCommandListD3D12* pCmdList = static_cast<GraphicsCommandListD3D12*>(cmdLists[i]); 
    pNativeLists[i] = pCmdList->getNativeList(m_frameIndex);
    if (pCmdList->isRecording()) {
      ASSERT(false && "Cmd list submitted for execution, when it is still in record mode!");
    }
  }

  pQueue->ExecuteCommandLists(numCmdLists, pNativeLists);
}


void D3D12Backend::signalFence(RendererT queue, Fence* fence)
{
  RendererT f = fence->getUUID();
  ID3D12CommandQueue* pQueue = m_pCommandQueues[queue];
  ID3D12Fence* pFence = m_fences[f];
  
  DX12ASSERT(pQueue->Signal(pFence, m_fenceValues[f]));
}


void D3D12Backend::waitFence(Fence* fence)
{
  RendererT f = fence->getUUID();
  ID3D12Fence* pFence = m_fences[f];

  pFence->SetEventOnCompletion(m_fenceValues[f], m_fenceEvents[f]);
  WaitForSingleObject(m_fenceEvents[f], INFINITE);  
}



void D3D12Backend::createDescriptorTable(DescriptorTable** table)
{
  DescriptorTableD3D12* pHeap = new DescriptorTableD3D12();
  *table = pHeap;
}


void D3D12Backend::createRootSignature(RootSignature** ppRootSig)
{
  RootSignatureD3D12* pRootSignature = new RootSignatureD3D12();
  *ppRootSig = pRootSignature;
}


void D3D12Backend::createVertexBufferView(VertexBufferView** ppView,
                                          Resource* pBuffer,
                                          U32 vertexStride,
                                          U32 bufferSzBytes)
{
  VertexBufferViewD3D12* pNativeView = new VertexBufferViewD3D12();
  *ppView = pNativeView;
  pNativeView->_buffer = pBuffer->getUUID();
  pNativeView->_szInBytes = bufferSzBytes;
  pNativeView->_vertexStrideBytes = vertexStride;
}


void D3D12Backend::createIndexBufferView(IndexBufferView** ppView,
                                         Resource* pBuffer,
                                         DXGI_FORMAT format,
                                         U32 szBytes)
{
  IndexBufferViewD3D12* pNativeView = new IndexBufferViewD3D12();
  *ppView = pNativeView;
  pNativeView->_buffer = pBuffer->getUUID();
  pNativeView->_format = format;
  pNativeView->_szBytes = szBytes;
}


void D3D12Backend::createFence(Fence** ppFence)
{
  Fence* pFence = new Fence();
  ID3D12Fence* pNativeFence = nullptr;
  DX12ASSERT(m_pDevice->CreateFence(0, 
                                    D3D12_FENCE_FLAG_NONE,
                                    __uuidof(ID3D12Fence), 
                                    (void**)&pNativeFence));
  *ppFence = pFence;
  m_fences[pFence->getUUID()] = pNativeFence;
  m_fenceEvents[pFence->getUUID()] = CreateEvent(NULL, FALSE, FALSE, NULL);
  m_fenceValues[pFence->getUUID()] = 1;
}


void D3D12Backend::destroyFence(Fence* pFence)
{
  m_fences[pFence->getUUID()]->Release();
  m_fences[pFence->getUUID()] = nullptr;
  delete pFence;
}


void D3D12Backend::createGraphicsPipelineState(GraphicsPipeline** ppPipeline,
                                               const GraphicsPipelineInfo* pInfo)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = { };

    desc.VS.BytecodeLength = pInfo->_vertexShader._szBytes;
    desc.VS.pShaderBytecode = pInfo->_vertexShader._pByteCode;

    desc.PS.BytecodeLength = pInfo->_pixelShader._szBytes;
    desc.PS.pShaderBytecode = pInfo->_pixelShader._pByteCode;

    desc.DS.BytecodeLength = pInfo->_domainShader._szBytes;
    desc.DS.pShaderBytecode = pInfo->_domainShader._pByteCode;


    desc.HS.BytecodeLength = pInfo->_hullShader._szBytes;
    desc.HS.pShaderBytecode = pInfo->_hullShader._pByteCode;

    desc.GS.BytecodeLength = pInfo->_geometryShader._szBytes;
    desc.GS.pShaderBytecode = pInfo->_geometryShader._pByteCode;
    
    desc.PrimitiveTopologyType = getNativeTopologyType(pInfo->_topology);
    desc.DSVFormat = pInfo->_dsvFormat;
    desc.NumRenderTargets = pInfo->_numRenderTargets;
    desc.NodeMask = 0;
    desc.pRootSignature = getBackendD3D12()->getRootSignature(pInfo->_pRootSignature->getUUID());
    
    processRasterizationState(desc, pInfo->_rasterizationState);
    processDepthStencilState(desc, pInfo->_depthStencilState);
    processBlendState(desc, pInfo->_blendState);

    desc.IBStripCutValue = getIBCutValue(pInfo->_ibCutValue);
    desc.StreamOutput;
    desc.SampleMask = pInfo->_sampleMask;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    for (U32 i = 0; i < 8; ++i) {
      desc.RTVFormats[i] = pInfo->_rtvFormats[i];
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> elementDescs(pInfo->_inputLayout._elementCount);
    for (U32 i = 0; i < elementDescs.size(); ++i) {
      elementDescs[i].AlignedByteOffset = pInfo->_inputLayout._pInputElements[i]._alignedByteOffset;
      elementDescs[i].Format = pInfo->_inputLayout._pInputElements[i]._format;
      elementDescs[i].InputSlot = pInfo->_inputLayout._pInputElements[i]._inputSlot;
      elementDescs[i].InputSlotClass = getD3D12InputClassification(pInfo->_inputLayout._pInputElements[i]._classification);
      elementDescs[i].InstanceDataStepRate = pInfo->_inputLayout._pInputElements[i]._instanceDataStepRate;
      elementDescs[i].SemanticIndex = pInfo->_inputLayout._pInputElements[i]._semanticIndex;
      elementDescs[i].SemanticName = pInfo->_inputLayout._pInputElements[i]._semanticName;
    }

    desc.InputLayout.NumElements = pInfo->_inputLayout._elementCount;
    desc.InputLayout.pInputElementDescs = elementDescs.data();

    ID3D12PipelineState* pPipelineState = nullptr;
  
    DX12ASSERT(m_pDevice->CreateGraphicsPipelineState(&desc, 
                                                      __uuidof(ID3D12PipelineState), 
                                                      (void**)&pPipelineState));
    GraphicsPipelineStateD3D12* pPipe = new GraphicsPipelineStateD3D12(); 
    *ppPipeline = pPipe;
    m_pPipelineStates[(*ppPipeline)->getUUID()] = pPipelineState;
    pPipe->_topology = getPrimitiveTopology(pInfo->_topology);
}


void D3D12Backend::createComputePipelineState(ComputePipeline** ppPipeline,
                                              const ComputePipelineInfo* pInfo)
{
    *ppPipeline = new ComputePipelineStateD3D12();

    D3D12_COMPUTE_PIPELINE_STATE_DESC compDesc = { };
    compDesc.pRootSignature = getRootSignature(pInfo->_pRootSignature->getUUID());
    compDesc.CS.BytecodeLength = pInfo->_computeShader._szBytes;
    compDesc.CS.pShaderBytecode = pInfo->_computeShader._pByteCode;

    ID3D12PipelineState* pPipelineState = nullptr;

  DX12ASSERT(m_pDevice->CreateComputePipelineState(&compDesc, 
                                                   __uuidof(ID3D12PipelineState), 
                                                   (void**)&pPipelineState));
  m_pPipelineStates[(*ppPipeline)->getUUID()] = pPipelineState;
}


void D3D12Backend::createRayTracingPipelineState(RayTracingPipeline** ppPipeline, const RayTracingPipelineInfo* pInfo)
{
    if (!m_hardwareRaytracingCompatible) {
        DEBUG("ERROR: This GPU is not compatible for hardware ray tracing! Skipping: ", __FUNCTION__);
    }

    // Capture all of our subobjects that we need for our ray tracing pipeline.
    // this actually sucks.
    std::vector<D3D12_STATE_SUBOBJECT> rayTracingSubobjects;

    ID3D12Device5* dxrDevice;
    m_pDevice->QueryInterface<ID3D12Device5>(&dxrDevice);
    D3D12_STATE_OBJECT_DESC rayTracingPipeline = { };
    rayTracingPipeline.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

    D3D12_DXIL_LIBRARY_DESC desc = { };
    D3D12_SHADER_BYTECODE byteCode = { };
    desc.DXILLibrary.BytecodeLength = byteCode.BytecodeLength;
    desc.DXILLibrary.pShaderBytecode = byteCode.pShaderBytecode;
    D3D12_STATE_SUBOBJECT lib;
    lib.pDesc = &desc;
    lib.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;

    U32 numShaders = 0;
    for (U32 i = 0; i < RAYTRACING_SHADER_END; ++i) {
        if (pInfo->shaderNames[i]) numShaders++;
    }

    std::vector<D3D12_EXPORT_DESC> exports(numShaders);
    desc.NumExports = numShaders;
    desc.pExports = exports.data();

    {
        U32 count = 0;
        for (U32 i = 0; i < RAYTRACING_SHADER_END; ++i) {
            if (pInfo->shaderNames[i])
                exports[count++].Name = pInfo->shaderNames[i];
            
        }
    }

    rayTracingSubobjects.push_back(lib);

    D3D12_HIT_GROUP_DESC hitGroup = { };
    hitGroup.Type = getHitGroupType(pInfo->hitGroupType);
    hitGroup.AnyHitShaderImport = pInfo->shaderNames[RAYTRACING_SHADER_ANYHIT];
    hitGroup.ClosestHitShaderImport = pInfo->shaderNames[RAYTRACING_SHADER_CLOSESTHIT];
    hitGroup.IntersectionShaderImport = pInfo->shaderNames[RAYTRACING_SHADER_INTERSECTION];
    hitGroup.HitGroupExport = pInfo->hitGroupName;

    D3D12_STATE_SUBOBJECT hitSubobj = { };
    hitSubobj.pDesc = &hitGroup;
    hitSubobj.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    rayTracingSubobjects.push_back(hitSubobj);

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = { };
    shaderConfig.MaxPayloadSizeInBytes = pInfo->payloadSzBytes;
    shaderConfig.MaxAttributeSizeInBytes = pInfo->attribSzBytes;
    D3D12_STATE_SUBOBJECT shaderconfSubobj = {};
    shaderconfSubobj.pDesc = &shaderConfig;
    shaderconfSubobj.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    rayTracingSubobjects.push_back(shaderconfSubobj);

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSig = { };
    globalRootSig.pGlobalRootSignature = getRootSignature( pInfo->pGlobalRootSignature->getUUID() );
    D3D12_STATE_SUBOBJECT globalRootSigSubobj = { };
    globalRootSigSubobj.pDesc = &globalRootSig;
    globalRootSigSubobj.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;

    // Make some associations.
    std::vector<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION> associations;
    std::vector<D3D12_LOCAL_ROOT_SIGNATURE> localRootSigs;
    std::vector<D3D12_STATE_SUBOBJECT> stateSubObjects;

    for (U32 i = 0; i < RAYTRACING_SHADER_END; ++i) {
        RootSignature* pLocalRootSig = pInfo->pLocalRootSignatures[i];
        if (!pLocalRootSig) continue;
        ID3D12RootSignature* pSignature = getRootSignature(pLocalRootSig->getUUID());
        {
            D3D12_LOCAL_ROOT_SIGNATURE localRootSig = { };
            localRootSig.pLocalRootSignature = pSignature;
            localRootSigs.push_back(localRootSig);
        }
        D3D12_STATE_SUBOBJECT subobj = { };
        subobj.pDesc = &localRootSigs.back();
        subobj.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
        stateSubObjects.push_back(subobj);

        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association = { };
        association.NumExports = 1;
        // TODO(): Need to test this.
        association.pExports = (LPCWSTR*)&pInfo->shaderNames[i];
        association.pSubobjectToAssociate = &stateSubObjects.back();
        associations.push_back(association);
    }


    rayTracingPipeline.NumSubobjects = rayTracingSubobjects.size();
    rayTracingPipeline.pSubobjects = rayTracingSubobjects.data();
    ID3D12StateObject* pRayTracingPipeline = nullptr;
    DX12ASSERT(dxrDevice->CreateStateObject(&rayTracingPipeline, 
                                            __uuidof(ID3D12StateObject), 
                                            (void**)&pRayTracingPipeline));

    RayTracingPipeline* pPipeline = new RayTracingPipeline();
    m_pStateObjects[pPipeline->getUUID()] = pRayTracingPipeline;
    *ppPipeline = pPipeline;
}

void D3D12Backend::createAccelerationStructure(Resource** ppResource,
                                const AccelerationStructureGeometry* geometryInfos,
                                U32 geometryCount,
                                const AccelerationStructureTopLevelInfo* pTopLevelInfo)
{
    ID3D12Device5* dxrDevice;
    m_pDevice->QueryInterface<ID3D12Device5>(&dxrDevice);

    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDescs(geometryCount);

    for (U32 i = 0; i < geometryCount; ++i) {
        D3D12_RAYTRACING_GEOMETRY_DESC& geom = geomDescs[i];
        geom = { };

        switch (geometryInfos[i]._type) 
        {   
            case RAYTRACING_HITGROUP_TYPE_PROCEDURAL_PRIMITIVE: 
            {
                ID3D12Resource* pResource = getResource(geometryInfos[i].aabbs._aabbResource->getUUID());
                geom.AABBs.AABBCount = geometryInfos[i].aabbs._count;
                
                geom.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
                geom.AABBs.AABBs.StartAddress = pResource->GetGPUVirtualAddress();
                geom.AABBs.AABBs.StrideInBytes = geometryInfos[i].aabbs.strideInBytes;
                break;
            }

            case RAYTRACING_HITGROUP_TYPE_TRIANGLES:
            default: 
            {
                ID3D12Resource* pVertexBuffer = getResource(geometryInfos[i]._tris._vertexBuffer->getUUID());
                ID3D12Resource* pIndexBuffer = getResource(geometryInfos[i]._tris._indexBuffer->getUUID());
                geom.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                geom.Triangles.IndexBuffer = pIndexBuffer->GetGPUVirtualAddress();
                geom.Triangles.IndexFormat = geometryInfos[i]._tris._indexFormat;
                geom.Triangles.IndexCount = geometryInfos[i]._tris._indexCount;
                // TODO: Add offsetting for this address if needed.
                geom.Triangles.VertexBuffer.StartAddress = pVertexBuffer->GetGPUVirtualAddress();
                geom.Triangles.VertexBuffer.StrideInBytes - geometryInfos[i]._tris.strideInBytes;
                geom.Triangles.VertexCount = geometryInfos[i]._tris._vertexCount;
                geom.Triangles.VertexFormat = geometryInfos[i]._tris._vertexFormat;

                geom.Triangles.Transform3x4 = 0;
                break;
            }
        }
    }

    // TODO: Allow build types to be used by the application.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = { };
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = { };

    {
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = { };
        topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        topLevelInputs.NumDescs = 1;
        topLevelInputs.pGeometryDescs = nullptr;
        topLevelInputs.Flags = buildFlags;
        dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    }
    
    {
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS input = { };
        input.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        input.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;    
        input.Flags = buildFlags;
        input.pGeometryDescs = geomDescs.data();
        input.NumDescs = geometryCount;
        dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&input, &bottomLevelPrebuildInfo);
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelAS = { };

    ID3D12Resource* pScratchBuffer = nullptr;
    D3D12_RESOURCE_DESC scratchDesc = { };
    scratchDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    scratchDesc.Alignment = 0;
    scratchDesc.Width = max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes);
    scratchDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    scratchDesc.Height = scratchDesc.DepthOrArraySize = 1;
    scratchDesc.Format = DXGI_FORMAT_UNKNOWN;
    scratchDesc.SampleDesc.Count = 1;
    scratchDesc.SampleDesc.Quality = 0;
    pCustomMemoryAllocator->allocate(m_pDevice, D3D12_HEAP_TYPE_CUSTOM, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, scratchDesc);
}


void D3D12Backend::createSampler(Sampler** ppSampler, const SamplerDesc* pDesc)
{
    D3D12_SAMPLER_DESC desc = { };
    desc.MaxAnisotropy = pDesc->_maxAnisotropy;
    desc.MinLOD = pDesc->_minLod;
    desc.MaxLOD = pDesc->_maxLod;
    desc.MipLODBias = pDesc->_mipLodBias;
    desc.ComparisonFunc = getComparisonFunc( pDesc->_comparisonFunc );
    desc.Filter = getNativeFilter(pDesc->_filter);
    desc.AddressU = getNativeTextureAddress(pDesc->_addressU);
    desc.AddressV = getNativeTextureAddress(pDesc->_addressV);
    desc.AddressW = getNativeTextureAddress(pDesc->_addressW);
    desc.BorderColor[0] = pDesc->_borderColor[0];
    desc.BorderColor[1] = pDesc->_borderColor[1];
    desc.BorderColor[2] = pDesc->_borderColor[2];
    desc.BorderColor[3] = pDesc->_borderColor[3];

    D3D12_CPU_DESCRIPTOR_HANDLE heapOffset = m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_SAMPLER];
    m_pDevice->CreateSampler(&desc, heapOffset);

    UINT atomSz = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    m_descriptorHeapCurrentOffset[DESCRIPTOR_HEAP_SAMPLER].ptr += atomSz;
    
    *ppSampler = new Sampler();
    m_samplers[(*ppSampler)->getUUID()] = heapOffset;
}


void D3D12Backend::destroySampler(Sampler* pSampler)
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = getSamplerDescriptorHandle(pSampler->getUUID());
    if (!cpuHandle.ptr) return;
    
    m_samplers[pSampler->getUUID()].ptr = 0;
    
    delete pSampler; 
}
} // gfx