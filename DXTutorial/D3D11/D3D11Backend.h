

#pragma once

#include "../Renderer.h"

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d11_3.h>
#include <d3d11_4.h>

#include <vector>
#include <unordered_map>

namespace gfx {


class D3D11Backend : public BackendRenderer
{
public:

    void initialize(HWND handle, const GraphicsConfiguration& configs) override;
    void cleanUp() override;

private:

    ID3D11Device2* m_pDevice;
};
} // gfx