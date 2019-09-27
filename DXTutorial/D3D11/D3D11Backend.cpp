#include "D3D11Backend.h"


namespace gfx {


void D3D11Backend::createBuffer(Buffer** buffer,
                                BufferUsage usage,
                                BufferBindFlags binds,
                                BufferDimension dimension,
                                U32 width,
                                U32 height,
                                U32 depth,
                                DXGI_FORMAT format)
{
    ID3D11Buffer* pNativeBuffer = nullptr;
    BufferD3D11* pBuffer = new BufferD3D11();
    *buffer = pBuffer;
    m_buffers[pBuffer->getUUID()] = pNativeBuffer;
}
} // gfx