//
#pragma once

#include "../Renderer.h"
#include "D3D12Backend.h"

namespace gfx {


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
  GraphicsPipelineStateD3D12(D3D12Backend* backend)
    : _pBackend(backend) { }

  void initialize(const GraphicsPipelineInfo* pInfo) override {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = { };
  }

  D3D12Backend* _pBackend;
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