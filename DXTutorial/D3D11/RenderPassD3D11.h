#pragma once

#include "../BackendRenderer.h"

#include "D3D11Backend.h"

namespace gfx {



class RenderPassD3D11 : public RenderPass
{
public:
  void setRenderTargets(RenderTargetView** ppRenderTargets, U32 renderTargetCount) override { 
  }

  void setDepthStencil(DepthStencilView* pDepthStencil) override {
      
  }

  std::vector<RenderTargetView*> m_pRenderTargets;
  DepthStencilView* pDepthStencilView;
};
}  // gfx