#pragma once

#include "CommonsD3D11.h"
#include "../Renderer.h"

namespace gfx {

class GraphicsCommandListImmediateD3D11 : public CommandList
{
public:

private:

};


class GraphicsCommandListDeferredD3D11 : public CommandList
{
public:

  void drawInstanced(U32 vertexCountPerInstance, 
                     U32 instanceCount, 
                     U32 startVertexLocation, 
                     U32 startInstanceLocation) override 
  {
    m_ctx->DrawInstanced(vertexCountPerInstance, instanceCount, startInstanceLocation, startInstanceLocation);
  }

  void close() override {
    m_ctx->FinishCommandList(FALSE, &m_pCmdList);
  }

private:
    ID3D11DeviceContext* m_ctx;
    ID3D11CommandList* m_pCmdList;
};
} // gfx