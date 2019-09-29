
#pragma once


#include "CommonsD3D12.h"
#include "../Renderer.h"
#include "D3D12Backend.h"


namespace gfx {


struct RenderPassD3D12 : public RenderPass {
  RenderPassD3D12() {}

  void setRenderTargets(RenderTargetView** rtvs, U32 rtvCount) override {
    _renderTargetResourceIds.resize(rtvCount);
    for (U32 i = 0; i < rtvCount; ++i) {
      _renderTargetResourceIds[i] = rtvs[i]->getUUID();
    }
  }

  void setDepthStencil(DepthStencilView* depthstencil) override {
    _depthStencilResourceId = depthstencil->getUUID();  
  }

  void finalize() override {};

  RendererT _depthStencilResourceId;
  std::vector<RendererT> _renderTargetResourceIds;
};
} // gfx