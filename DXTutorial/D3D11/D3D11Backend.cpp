#include "D3D11Backend.h"
#include "CommandListD3D11.h"

namespace gfx {


U32 getNativeBindFlags(BufferBindFlags binds)
{
  U32 flags = 0;
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


void* BufferD3D11::map(U64 start, U64 end)
{
  ID3D11Buffer* pBuffer = _pBackend->getBuffer(getUUID());
  D3D11_MAPPED_SUBRESOURCE mapped = { };
  HRESULT result = _pBackend->getImmediateCtx()->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped); 
  DX11ASSERT(result);
  return mapped.pData;
}


void BufferD3D11::unmap(U64 start, U64 end)
{
  ID3D11Buffer* pBuffer = _pBackend->getBuffer(getUUID());
  _pBackend->getImmediateCtx()->Unmap(pBuffer, 0);
}


void D3D11Backend::initialize(HWND handle, bool isFullScreen, const GpuConfiguration& configs)
{
  IDXGIFactory2* pFactory = createFactory();
  createDevice(pFactory);
  createSwapchain(handle, 
                  pFactory, 
                  configs._renderWidth, 
                  configs._renderHeight, 
                  configs._desiredBuffers, 
                  configs._windowed);
  queryFromSwapchain();
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
      DXGI_ADAPTER_DESC1 adapterDesc = { };
      DX11ASSERT(pTempAdapter->GetDesc1(&adapterDesc));
      if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        continue;
      }
      pChoosenAdapter = pTempAdapter;
      break;
  }

    D3D_FEATURE_LEVEL feature = D3D_FEATURE_LEVEL_11_1;
    UINT flags = 0;
#if _DEBUG
    flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT result = D3D11CreateDevice(pTempAdapter, 
                                 D3D_DRIVER_TYPE_UNKNOWN, 
                                 nullptr, 
                                 flags, 
                                 &feature, 
                                 1, 
                                 D3D11_SDK_VERSION, 
                                 &m_pDevice, 
                                 nullptr, 
                                 &m_pImmediateCtx);
  DX11ASSERT(result);
}


void D3D11Backend::createSwapchain(HWND hWnd, 
                                  IDXGIFactory2* pFactory, 
                                  U32 renderWidth, 
                                  U32 renderHeight,
                                  U32 desiredBuffers, 
                                  B32 windowed)
{
  DXGI_SWAP_CHAIN_DESC1 desc = { };
  desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
  desc.BufferCount = 3;desiredBuffers;
  desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.Height = renderHeight;
  desc.Width = renderWidth;
  desc.Scaling = DXGI_SCALING_STRETCH;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Stereo = FALSE;
  DX11ASSERT(pFactory->CreateSwapChainForHwnd(m_pDevice,
                                              hWnd,
                                              &desc,
                                              nullptr,
                                              nullptr,
                                              &m_pSwapChain));
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

    if (usage == BUFFER_USAGE_DEFAULT) {
      bufferDesc.CPUAccessFlags = 0;
      bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    }
    pBuffer->_usage = usage;
    pBuffer->_format = format;
    pBuffer->_dimension = dimension;
    pBuffer->_width = width;
    pBuffer->_height = height;
    pBuffer->_depth = depth;
    pBuffer->_flags = binds;
    pBuffer->_pBackend = this;
 
    HRESULT result = m_pDevice->CreateBuffer(&bufferDesc, 
                                             nullptr,
                                             &pNativeBuffer); 
    DX11ASSERT(result);
    m_buffers[pBuffer->getUUID()] = pNativeBuffer;
}


void D3D11Backend::createCommandList(CommandList** pList)
{
  GraphicsCommandListD3D11* pNativeList = nullptr;
  pNativeList = new GraphicsCommandListD3D11(this);

  *pList = pNativeList;
}


void D3D11Backend::submit(RendererT queue, CommandList** cmdLists, U32 numCmdLists)
{
  (void) queue;
  for (U32 i = 0; i < numCmdLists; ++i) {
    ID3D11CommandList* pCmd = static_cast<GraphicsCommandListD3D11*>(cmdLists[i])->m_pCmdList;
    m_pImmediateCtx->ExecuteCommandList(pCmd, FALSE);
  }
}


void D3D11Backend::queryFromSwapchain()
{
  DXGI_SWAP_CHAIN_DESC desc = { };
  m_pSwapChain->GetDesc(&desc);

  HRESULT result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_frameResource._surface);
  DX11ASSERT(result);
  ID3D11RenderTargetView* pRtv = nullptr;
  D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = { };
  rtvDesc.Format = desc.BufferDesc.Format;
  rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  rtvDesc.Texture2D.MipSlice = 0;
  result = m_pDevice->CreateRenderTargetView(m_frameResource._surface, 
                                                      0, 
                                                      &pRtv); 
  DX11ASSERT(result);
  m_renderTargetViews[m_frameResource._view.getUUID()] = pRtv;
}


void D3D11Backend::present()
{
  m_pSwapChain->Present(1, 0);
}

void D3D11Backend::createRenderTargetView(RenderTargetView** rtv, Buffer* buffer)
{
    ID3D11RenderTargetView* pNativeView = nullptr;
    TargetView* pView = new TargetView();
    
    BufferD3D11*  pBuffer = static_cast<BufferD3D11*>(buffer);
    ID3D11Buffer* pNative = m_buffers[buffer->getUUID()];
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = { };
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
    rtvDesc.Buffer.ElementWidth = pBuffer->_width * pBuffer->_height * pBuffer->_depth;
    rtvDesc.Buffer.FirstElement = 0;
    //rtvDesc.Buffer.NumElements = ;
    rtvDesc.Format = pBuffer->_format;

    HRESULT result = m_pDevice->CreateRenderTargetView(pNative, &rtvDesc, &pNativeView);
    DX11ASSERT(result);

    m_renderTargetViews[pView->getUUID()] = pNativeView;
    *rtv = pView;
}
} // gfx