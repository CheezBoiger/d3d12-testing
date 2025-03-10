
#pragma once


#include "../BackendRenderer.h"
#include "D3D11Backend.h"

#include <vector>

namespace gfx {


struct DescriptorTableD3D11 : public DescriptorTable
{

  void setShaderResourceViews(ShaderResourceView** resources, U32 bufferCount) override {}
  void setUnorderedAccessViews(UnorderedAccessView** uavs, U32 uavCount) override {}
  
  void setConstantBuffers(Resource** buffer, U32 bufferCount) override {
    _constantBuffers.resize(bufferCount);
    for (U32 i = 0; i < bufferCount; ++i) {
      _constantBuffers[i] = buffer[i];
    }
  }
  
  void setSamplers(Sampler** samplers, U32 samplerCount) override {}
  void initialize(DescriptorTableType type, U32 totalCount) override { }
  virtual void update(DescriptorTableFlags flags) {}

  std::vector<Resource*> _constantBuffers;
  std::vector<Resource*> _shaderResourceViews;
  std::vector<Resource*> _unorderedAccessViews;
};
} // gfx