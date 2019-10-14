
#pragma once


#include "CommonsD3D12.h"
#include "../BackendRenderer.h"
#include "D3D12Backend.h"


namespace gfx {


struct RenderPassD3D12 : public RenderPass {
  RenderPassD3D12() {}

  void setRenderTargets(RenderTargetView** rtvs, U32 rtvCount) override {
    _renderTargetViews.resize(rtvCount);
    for (U32 i = 0; i < rtvCount; ++i) {
      _renderTargetViews[i] = rtvs[i];
    }
  }

  void setDepthStencil(DepthStencilView* depthstencil) override {
    _depthStencilResourceId = depthstencil;
  }

  void finalize() override {};

  DepthStencilView* _depthStencilResourceId;
  std::vector<RenderTargetView*> _renderTargetViews;
};
} // gfx