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

private:
    ID3D11DeviceContext* m_ctx;
    ID3D11CommandList* m_pCmdList;
};
} // gfx