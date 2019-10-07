//
#pragma once

#include "../Renderer.h"
#include "D3D12Backend.h"

namespace gfx {


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
    case CULL_MODE_FRONT: return D3D12_CULL_MODE_FRONT;
    case CULL_MODE_NONE: return D3D12_CULL_MODE_NONE;
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


void processRasterizationState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, 
                               const RasterizationState& rasterState)
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
  desc.RasterizerState.ForcedSampleCount;
  desc.RasterizerState.FrontCounterClockwise;
  desc.RasterizerState.SlopeScaledDepthBias;
}


void processDepthStencilState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
                              const DepthStencilState& dsState)
{
}


void processBlendState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
                       const BlendState& blendState)
{
}


void processInputLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
                        const InputLayout& layout)
{
}


D3D12_ROOT_SIGNATURE_FLAGS getNativeRootSignatureFlags(ShaderVisibilityFlags flags)
{
  D3D12_ROOT_SIGNATURE_FLAGS v = D3D12_ROOT_SIGNATURE_FLAG_NONE;
  if (!(flags & SHADER_VISIBILITY_DOMAIN)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
  }
  if (!(flags & SHADER_VISIBILITY_HULL)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
  }
  if (!(flags & SHADER_VISIBILITY_VERTEX)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
  }
  if (!(flags & SHADER_VISIBILITY_PIXEL)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
  }
  if (!(flags & SHADER_VISIBILITY_GEOMETRY)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
  }
  return v;
}


class GraphicsPipelineStateD3D12 : public GraphicsPipeline
{
  GraphicsPipelineStateD3D12() { }

  void initialize(const GraphicsPipelineInfo* pInfo) override {
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
    processInputLayout(desc, pInfo->_inputLayout);

    desc.IBStripCutValue = getIBCutValue(pInfo->_ibCutValue);
    desc.StreamOutput;
    desc.SampleMask = pInfo->_sampleMask;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    for (U32 i = 0; i < 8; ++i) {
      desc.RTVFormats[i] = pInfo->_rtvFormats[i];
    }


    ID3D12PipelineState* pPipelineState = nullptr;
  
    DX12ASSERT(getBackendD3D12()->getDevice()->CreateGraphicsPipelineState(&desc, 
                                                                           __uuidof(ID3D12PipelineState), 
                                                                           (void**)&pPipelineState));
    getBackendD3D12()->setPipelineState(getUUID(), pPipelineState);
  }

};


class ComputePipelineStateD3D12 : public ComputePipeline
{
  ComputePipelineStateD3D12(D3D12Backend* backend)
    : _pBackend(backend) { }


  void initialize(const ComputePipelineInfo* pInfo) override {
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = { };
  }

  D3D12Backend* _pBackend;
};
} //