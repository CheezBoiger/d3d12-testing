#pragma once

#include "../Renderer.h"
#include "D3D11Backend.h"

namespace gfx {


class RootSignatureD3D11 : public RootSignature
{
  void initialize(ShaderVisibilityFlags flags, 
                  PipelineLayout* pLayouts, 
                  U32 numLayouts) override {
    layouts.resize(numLayouts);
    for (U32 i = 0; i < numLayouts; ++i) {
      layouts[i] = pLayouts[i];
    }

    visibleFlags = flags;
  }

  std::vector<PipelineLayout> layouts;
  ShaderVisibilityFlags visibleFlags;
};
} // gfx