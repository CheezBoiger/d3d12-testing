#include "D3D11Backend.h"


namespace gfx {


U32 getNativeBindFlags(BufferBindFlags binds)
{
  U32 flags;
  if (binds & BUFFER_BIND_RENDER_TARGET)
    flags |= D3D11_BIND_RENDER_TARGET;
  if (binds & BUFFER_BIND_SHADER_RESOURCE)
    flags |= D3D11_BIND_SHADER_RESOURCE;
  if (binds & BUFFER_BIND_UNORDERED_ACCESS)
    flags |= D3D11_BIND_UNORDERED_ACCESS;
  if (binds & BUFFER_BIND_DEPTH_STENCIL)
    flags |= D3D11_BIND_DEPTH_STENCIL;
  if (binds & BUFFER_BIND_CONSTANT_BUFFER)
    flags |= D3D11_BIND_CONSTANT_BUFFER;
  if (binds & BUFFER_BIND_INDEX_BUFFER)
    flags |= D3D11_BIND_INDEX_BUFFER;
  if (flags & BUFFER_BIND_VERTEX_BUFFER)
    flags |= D3D11_BIND_VERTEX_BUFFER;
  return flags;
}


void D3D11Backend::initialize(HWND handle, bool isFullScreen, const GpuConfiguration& configs)
{
  IDXGIFactory2* pFactory = createFactory();
  createDevice(pFactory);

}


IDXGIFactory2* D3D11Backend::createFactory()
{
  IDXGIFactory2* pFactory = nullptr;
  HRESULT result = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), (void**)&pFactory);
  if (FAILED(result)) {
    DX11ASSERT(result);
  }
  return pFactory;
}


void D3D11Backend::createDevice(IDXGIFactory2* pFactory)
{
  IDXGIAdapter1* pTempAdapter = nullptr;
  IDXGIAdapter1* pChoosenAdapter = nullptr;
  for (U32 i = 0; 
       pFactory->EnumAdapters1(i, &pTempAdapter) != DXGI_ERROR_INVALID_CALL; 
       ++i) 
  {
      
  }
}


void D3D11Backend::createBuffer(Buffer** buffer,
                                BufferUsage usage,
                                BufferBindFlags binds,
                                BufferDimension dimension,
                                U32 width,
                                U32 height,
                                U32 depth,
                                U32 structureByteStride,
                                DXGI_FORMAT format,
                                const TCHAR* debugName)
{
    ID3D11Buffer* pNativeBuffer = nullptr;
    BufferD3D11* pBuffer = new BufferD3D11();
    *buffer = pBuffer;
    D3D11_BUFFER_DESC bufferDesc = { };

    bufferDesc.BindFlags = getNativeBindFlags(binds);
    bufferDesc.ByteWidth = width * height * depth;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.StructureByteStride = structureByteStride;
    
    if (usage == BUFFER_USAGE_GPU_TO_CPU) {
      bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
      bufferDesc.Usage = D3D11_USAGE_STAGING;
    }
    
    if (usage == BUFFER_USAGE_CPU_TO_GPU) {
      bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    }
    pBuffer->_format = format;
    pBuffer->_dimension = dimension;
    pBuffer->_width = width;
    pBuffer->_height = height;
    pBuffer->_depth = depth;
    pBuffer->_flags = binds;
 
    DX11ASSERT(m_pDevice->CreateBuffer(&bufferDesc, nullptr, &pNativeBuffer));
    m_buffers[pBuffer->getUUID()] = pNativeBuffer;
}
} // gfx