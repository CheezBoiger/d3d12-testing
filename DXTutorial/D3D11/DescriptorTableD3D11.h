
#pragma once


#include "../BackendRenderer.h"
#include "D3D11Backend.h"

#include <vector>

namespace gfx {


struct DescriptorTableD3D11 : public DescriptorTable
{

  void setShaderResourceViews(Resource** resources, U32 bufferCount) override {}
  void setUnorderedAccessViews(Resource** resources, U32 bufferCount) override {}
  
  void setConstantBuffers(Resource** buffer, U32 bufferCount) override {
    _constantBuffers.resize(bufferCount);
    for (U32 i = 0; i < bufferCount; ++i) {
      _constantBuffers[i] = buffer[i];
    }
  }
  
  void setSamplers(Sampler** samplers, U32 samplerCount) override {}
  void finalize() override { isFinalized = true; }
  virtual void update() {}

  std::vector<Resource*> _constantBuffers;
  std::vector<Resource*> _shaderResourceViews;
  std::vector<Resource*> _unorderedAccessViews;
  B32 isFinalized;
};
} // gfx