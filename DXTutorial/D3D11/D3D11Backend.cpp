#include "D3D11Backend.h"
#include "CommandListD3D11.h"
#include "DescriptorTableD3D11.h"
#include "RenderPassD3D11.h"

namespace gfx {


D3D11Backend* getBackendD3D11()
{
  static D3D11Backend backend;
  return &backend;
}

U32 getNativeBindFlags(ResourceBindFlags binds)
{
  U32 flags = 0;
  if (binds & RESOURCE_BIND_RENDER_TARGET)
    flags |= D3D11_BIND_RENDER_TARGET;
  if (binds & RESOURCE_BIND_SHADER_RESOURCE)
    flags |= D3D11_BIND_SHADER_RESOURCE;
  if (binds & RESOURCE_BIND_UNORDERED_ACCESS)
    flags |= D3D11_BIND_UNORDERED_ACCESS;
  if (binds & RESOURCE_BIND_DEPTH_STENCIL)
    flags |= D3D11_BIND_DEPTH_STENCIL;
  if (binds & RESOURCE_BIND_CONSTANT_BUFFER)
    flags |= D3D11_BIND_CONSTANT_BUFFER;
  if (binds & RESOURCE_BIND_INDEX_BUFFER)
    flags |= D3D11_BIND_INDEX_BUFFER;
  if (binds & RESOURCE_BIND_VERTEX_BUFFER)
    flags |= D3D11_BIND_VERTEX_BUFFER;
  return flags;
}


void getNativeAccessAndUsage(ResourceUsage usage,
                             UINT& cpuAccess,
                             D3D11_USAGE& outUsage) {
  if (usage == RESOURCE_USAGE_GPU_TO_CPU) {
    cpuAccess = D3D11_CPU_ACCESS_READ;
    outUsage = D3D11_USAGE_STAGING;
  }

  if (usage == RESOURCE_USAGE_CPU_TO_GPU) {
    cpuAccess = D3D11_CPU_ACCESS_WRITE;
    outUsage = D3D11_USAGE_DYNAMIC;
  }

  if (usage == RESOURCE_USAGE_DEFAULT) {
    cpuAccess = 0;
    outUsage = D3D11_USAGE_DEFAULT;
  }
}


D3D11_BLEND_OP getD3D11BlendOp(BlendOp op)
{
  switch (op) {
    case BLEND_OP_SUBTRACT: return D3D11_BLEND_OP_SUBTRACT;
    case BLEND_OP_REV_SUBTRACT: return D3D11_BLEND_OP_REV_SUBTRACT;
    case BLEND_OP_MIN: return D3D11_BLEND_OP_MIN;
    case BLEND_OP_MAX: return D3D11_BLEND_OP_MAX;
    case BLEND_OP_ADD: 
    default: return D3D11_BLEND_OP_ADD;
  }
}


D3D11_BLEND getD3D11Blend(Blend blend)
{
  switch (blend) {
    case BLEND_ONE: return D3D11_BLEND_ONE;
    case BLEND_SRC_COLOR: return D3D11_BLEND_SRC_COLOR;
    case BLEND_INV_SRC_COLOR: return D3D11_BLEND_INV_SRC_COLOR;
    case BLEND_SRC_ALPHA: return D3D11_BLEND_SRC_ALPHA;
    case BLEND_INV_SRC_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
    case BLEND_DEST_ALPHA: return D3D11_BLEND_DEST_ALPHA;
    case BLEND_INV_DEST_ALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
    case BLEND_DEST_COLOR: return D3D11_BLEND_DEST_COLOR;
    case BLEND_INV_DEST_COLOR: return D3D11_BLEND_INV_DEST_COLOR;
    case BLEND_SRC_ALPHA_SAT: return D3D11_BLEND_SRC_ALPHA_SAT;
    case BLEND_BLEND_FACTOR: return D3D11_BLEND_BLEND_FACTOR;
    case BLEND_INV_BLEND_FACTOR: return D3D11_BLEND_INV_BLEND_FACTOR;
    case BLEND_SRC1_COLOR: return D3D11_BLEND_SRC1_COLOR;
    case BLEND_INV_SRC1_COLOR: return D3D11_BLEND_INV_SRC1_COLOR;
    case BLEND_SRC1_ALPHA: return D3D11_BLEND_SRC1_ALPHA;
    case BLEND_INV_SRC1_ALPHA: return D3D11_BLEND_INV_SRC1_ALPHA;
    case BLEND_ZERO:
    default: return D3D11_BLEND_ZERO;
  }
}


D3D11_LOGIC_OP getD3D11LogicOp(LogicOp op)
{
  switch (op) {
    case LOGIC_OP_SET: return D3D11_LOGIC_OP_SET;
    case LOGIC_OP_COPY: return D3D11_LOGIC_OP_COPY;
    case LOGIC_OP_COPY_INVERTED: return D3D11_LOGIC_OP_COPY_INVERTED;
    case LOGIC_OP_NOOP: return D3D11_LOGIC_OP_NOOP;
    case LOGIC_OP_INVERT: return D3D11_LOGIC_OP_INVERT;
    case LOGIC_OP_AND: return D3D11_LOGIC_OP_AND;
    case LOGIC_OP_NAND: return D3D11_LOGIC_OP_NAND;
    case LOGIC_OP_OR: return D3D11_LOGIC_OP_OR;
    case LOGIC_OP_NOR: return D3D11_LOGIC_OP_NOR;
    case LOGIC_OP_XOR: return D3D11_LOGIC_OP_XOR;
    case LOGIC_OP_EQUIV: return D3D11_LOGIC_OP_EQUIV;
    case LOGIC_OP_AND_REVERSE: return D3D11_LOGIC_OP_AND_REVERSE;
    case LOGIC_OP_AND_INVERTED: return D3D11_LOGIC_OP_AND_INVERTED;
    case LOGIC_OP_OR_REVERSE: return D3D11_LOGIC_OP_OR_REVERSE;
    case LOGIC_OP_OR_INVERTED: return D3D11_LOGIC_OP_OR_INVERTED;
    case LOGIC_OP_CLEAR:
    default: return D3D11_LOGIC_OP_CLEAR;
  }
}


D3D11_STENCIL_OP getD3D11StencilOp(StencilOp op)
{
  switch (op) {
    case STENCIL_OP_KEEP: return D3D11_STENCIL_OP_KEEP;
    case STENCIL_OP_REPLACE: return D3D11_STENCIL_OP_REPLACE;
    case STENCIL_OP_INCR_SAT: return D3D11_STENCIL_OP_INCR_SAT;
    case STENCIL_OP_DECR_SAT: return D3D11_STENCIL_OP_DECR_SAT;
    case STENCIL_OP_INVERT: return D3D11_STENCIL_OP_INVERT;
    case STENCIL_OP_INCR: return D3D11_STENCIL_OP_INCR;
    case STENCIL_OP_DECR: return D3D11_STENCIL_OP_DECR;
    case STENCIL_OP_ZERO: 
    default: return D3D11_STENCIL_OP_ZERO;
  }
}


D3D11_COMPARISON_FUNC getD3D11ComparisonFunc(ComparisonFunc func)
{
  switch (func) {
    case COMPARISON_FUNC_LESS: return D3D11_COMPARISON_LESS;
    case COMPARISON_FUNC_EQUAL: return D3D11_COMPARISON_EQUAL;
    case COMPARISON_FUNC_LESS_EQUAL: return D3D11_COMPARISON_LESS_EQUAL;
    case COMPARISON_FUNC_GREATER: return D3D11_COMPARISON_GREATER;
    case COMPARISON_FUNC_NOT_EQUAL: return D3D11_COMPARISON_NOT_EQUAL;
    case COMPARISON_FUNC_GREATER_EQUAL: return D3D11_COMPARISON_GREATER_EQUAL;
    case COMPARISON_FUNC_ALWAYS: return D3D11_COMPARISON_ALWAYS;
    case COMPARISON_FUNC_NEVER: 
    default: return D3D11_COMPARISON_NEVER; 
  }
}


D3D11_DEPTH_WRITE_MASK getD3D11WriteMask(DepthWriteMask mask)
{
  switch (mask) {
    case DEPTH_WRITE_MASK_ALL: return D3D11_DEPTH_WRITE_MASK_ALL;
    case DEPTH_WRITE_MASK_ZERO: 
    default: D3D11_DEPTH_WRITE_MASK_ZERO;
  }
}


D3D11_INPUT_CLASSIFICATION getD3D11InputClassification(InputClassification c)
{
  switch (c) {
    case INPUT_CLASSIFICATION_PER_INSTANCE: return D3D11_INPUT_PER_INSTANCE_DATA;
    case INPUT_CLASSIFICATION_PER_VERTEX:
    default: return D3D11_INPUT_PER_VERTEX_DATA;
  }
}


D3D11_PRIMITIVE_TOPOLOGY getD3D11PrimitiveTopology(PrimitiveTopology topo)
{
  switch (topo) {
    case PRIMITIVE_TOPOLOGY_LINES: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case PRIMITIVE_TOPOLOGY_PATCHES: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    case PRIMITIVE_TOPOLOGY_POINTS: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    case PRIMITIVE_TOPOLOGY_TRIANGLES: 
    default:
      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  }
}


D3D11_CULL_MODE getD3D11CullMode(CullMode mode)
{
  switch (mode) {
    case CULL_MODE_BACK: return D3D11_CULL_BACK;
    case CULL_MODE_FRONT: return D3D11_CULL_FRONT;
    case CULL_MODE_NONE: 
    default: return D3D11_CULL_NONE;
  }
}


D3D11_FILL_MODE getD3D11FillMode(FillMode fill)
{
  switch (fill) {
    case FILL_MODE_WIREFRAME: return D3D11_FILL_WIREFRAME;
    case FILL_MODE_SOLID:
    default: return D3D11_FILL_SOLID;
  }
}


D3D11_RENDER_TARGET_VIEW_DESC processRenderTargetViewDesc(const RenderTargetViewDesc& desc)
{
    D3D11_RENDER_TARGET_VIEW_DESC rtv;
    rtv.Format = desc._format;
    switch (desc._dimension) {
        case RESOURCE_DIMENSION_BUFFER:
            { 
                rtv.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
                rtv.Buffer.FirstElement =  desc._buffer._firstElement;
                rtv.Buffer.NumElements = desc._buffer._numElements;
            } break;
        case RESOURCE_DIMENSION_1D:
            {
                rtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
                rtv.Texture1D.MipSlice = desc._texture1D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_1D_ARRAY:
            {
                rtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                rtv.Texture1DArray.ArraySize = desc._texture1DArray._arraySize;
                rtv.Texture1DArray.FirstArraySlice = desc._texture1DArray._firstArraySlice;
                rtv.Texture1DArray.MipSlice = desc._texture1DArray._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D:
            {
                rtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtv.Texture2D.MipSlice = desc._texture2D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D_ARRAY:
            {
                rtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtv.Texture2DArray.ArraySize = desc._texture2DArray._arraySize;
                rtv.Texture2DArray.FirstArraySlice = desc._texture2DArray._firstArraySlice;
                rtv.Texture2DArray.MipSlice = desc._texture2DArray._mipSlice;
            } break;
        case RESOURCE_DIMENSION_3D:
            {
                rtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                rtv.Texture3D.FirstWSlice = desc._texture3D._firstWSlice;
                rtv.Texture3D.MipSlice = desc._texture3D._mipSlice;
                rtv.Texture3D.WSize = desc._texture3D._wSize;
            } break;
        default: break;
    }
    return rtv;
}

UINT processDSVFlags(U32 flags)
{
    UINT nativeFlags = 0;
    if (flags & DEPTH_STENCIL_FLAG_READ_ONLY_DEPTH) nativeFlags |= D3D11_DSV_READ_ONLY_DEPTH;
    if (flags & DEPTH_STENCIL_FLAG_READ_ONLY_STENCIL) nativeFlags |= D3D11_DSV_READ_ONLY_STENCIL;
    return nativeFlags;
}

D3D11_DEPTH_STENCIL_VIEW_DESC processDepthStencilViewDesc(const DepthStencilViewDesc& desc)
{
    D3D11_DEPTH_STENCIL_VIEW_DESC dsv;
    dsv.Format = desc._format;
    dsv.Flags = processDSVFlags(desc._flags);
    switch (desc._dimension) {
        case RESOURCE_DIMENSION_1D:
            {
                dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
                dsv.Texture1D.MipSlice = desc._texture1D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_1D_ARRAY:
            {
                dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
                dsv.Texture1DArray.ArraySize = desc._texture1DArray._arraySize;
                dsv.Texture1DArray.FirstArraySlice = desc._texture1DArray._firstArraySlice;
                dsv.Texture1DArray.MipSlice = desc._texture1DArray._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D:
            {
                dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                dsv.Texture2D.MipSlice = desc._texture2D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D_ARRAY:
            {
                dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                dsv.Texture2DArray.ArraySize = desc._texture2DArray._arraySize;
                dsv.Texture2DArray.FirstArraySlice = desc._texture2DArray._firstArraySlice;
                dsv.Texture2DArray.MipSlice = desc._texture2DArray._mipSlice;
            } break;
        default: break;
    }
    return dsv;
}


D3D11_SHADER_RESOURCE_VIEW_DESC processShaderResourceViewDesc(const ShaderResourceViewDesc& desc)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srv;
    srv.Format = desc._format;
    switch (desc._dimension) {
        case RESOURCE_DIMENSION_BUFFER:
            {
                srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
                srv.Buffer.FirstElement = desc._buffer._firstElement;
                srv.Buffer.NumElements = desc._buffer._numElements;
            } break;
        case RESOURCE_DIMENSION_1D: 
            {
                srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
                srv.Texture1D.MipLevels = desc._texture1D._mipLevels;
                srv.Texture1D.MostDetailedMip = desc._texture1D._mostDetailedMip;
            } break;
        case RESOURCE_DIMENSION_1D_ARRAY:
            {
                srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
                srv.Texture1DArray.ArraySize = desc._texture1DArray._arraySize;
                srv.Texture1DArray.FirstArraySlice = desc._texture1DArray._firstArraySlice;
                srv.Texture1DArray.MipLevels = desc._texture1DArray._mipLevels;
                srv.Texture1DArray.MostDetailedMip = desc._texture1DArray._mostDetailedMip;
            } break;
        case RESOURCE_DIMENSION_2D:
            {
                srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srv.Texture2D.MipLevels = desc._texture2D._mipLevels;
                srv.Texture2D.MostDetailedMip = desc._texture2D._mostDetailedMip;
            } break;
        case RESOURCE_DIMENSION_2D_ARRAY:
            {
                srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srv.Texture2DArray.ArraySize = desc._texture2DArray._arraySize;
                srv.Texture2DArray.FirstArraySlice = desc._texture2DArray._firstArraySlice;
                srv.Texture2DArray.MipLevels = desc._texture2DArray._mipLevels;
                srv.Texture2DArray.MostDetailedMip = desc._texture2DArray._mostDetailedMip;
            } break;
        case RESOURCE_DIMENSION_3D:
            {
                srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
                srv.Texture3D.MipLevels = desc._texture3D._mipLevels;
                srv.Texture3D.MostDetailedMip = desc._texture3D._mostDetailedMip;
            } break;
        case RESOURCE_DIMENSION_TEXTURE_CUBE:
            {
                srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                srv.TextureCube.MipLevels = desc._textureCube._mipLevels;
                srv.TextureCube.MostDetailedMip = desc._textureCube._mostDetailedMip;
            } break;
        case RESOURCE_DIMENSION_TEXTURE_CUBE_ARRAY:
            {
                srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
                srv.TextureCubeArray.First2DArrayFace = desc._textureCubeArray._first2DArrayFace;
                srv.TextureCubeArray.MipLevels = desc._textureCubeArray._mipLevels;
                srv.TextureCubeArray.MostDetailedMip = desc._textureCubeArray._mostDetailedMip;
                srv.TextureCubeArray.NumCubes = desc._textureCubeArray._numCubes;
            } break;
        default: break;
    }
    return srv;
}

UINT processUAVFlags(U32 flags)
{
    UINT nativeFlags = 0;
    if (flags & BUFFER_UAV_FLAG_RAW) nativeFlags |= D3D11_BUFFER_UAV_FLAG_RAW;
    return nativeFlags;
}

D3D11_UNORDERED_ACCESS_VIEW_DESC processUnorderedAccessViewDesc(const UnorderedAccessViewDesc& desc)
{
    D3D11_UNORDERED_ACCESS_VIEW_DESC uav;
    uav.Format = desc._format;
    switch (desc._dimension) {
        case RESOURCE_DIMENSION_BUFFER:
            {
                uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
                uav.Buffer.FirstElement = desc._buffer._firstElement;
                uav.Buffer.NumElements = desc._buffer._numElements;
                uav.Buffer.Flags = processUAVFlags(desc._buffer._flags);
            } break;
        case RESOURCE_DIMENSION_1D:
            {
                uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
                uav.Texture1D.MipSlice = desc._texture1D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_1D_ARRAY:
            {
                uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
                uav.Texture1DArray.ArraySize = desc._texture1DArray._arraySize;
                uav.Texture1DArray.FirstArraySlice = desc._texture1DArray._firstArraySlice;
                uav.Texture1DArray.MipSlice = desc._texture1DArray._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D:
            {
                uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                uav.Texture2D.MipSlice = desc._texture2D._mipSlice;
            } break;
        case RESOURCE_DIMENSION_2D_ARRAY:
            {
                uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                uav.Texture2DArray.ArraySize = desc._texture2DArray._arraySize;
                uav.Texture2DArray.FirstArraySlice = desc._texture2DArray._firstArraySlice;
                uav.Texture2DArray.MipSlice = desc._texture2DArray._mipSlice;
            } break;
        case RESOURCE_DIMENSION_3D:
            {
                uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
                uav.Texture3D.FirstWSlice = desc._texture3D._firstWSlice;
                uav.Texture3D.MipSlice = desc._texture3D._mipSlice;
                uav.Texture3D.WSize = desc._texture3D._wSize;
            } break;
        default: break;
    }
    return uav;
}


void* BufferD3D11::map(const ResourceMappingRange* pRange)
{
  ID3D11Buffer* pBuffer = static_cast<ID3D11Buffer*>(_pBackend->getResource(getUUID()));
  D3D11_MAPPED_SUBRESOURCE mapped = { };
  HRESULT result = _pBackend->getImmediateCtx()->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped); 
  DX11ASSERT(result);
  return mapped.pData;
}


void BufferD3D11::unmap(const ResourceMappingRange* pRange)
{
  ID3D11Buffer* pBuffer = static_cast<ID3D11Buffer*>(_pBackend->getResource(getUUID()));
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


void D3D11Backend::createBuffer(Resource** buffer,
                                ResourceUsage usage,
                                ResourceBindFlags binds,
                                U32 widthBytes,
                                U32 structureByteStride,
                                const TCHAR* debugName)
{
    ID3D11Buffer* pNativeBuffer = nullptr;
    BufferD3D11* pBuffer = new BufferD3D11(RESOURCE_DIMENSION_BUFFER, usage, binds);
    *buffer = pBuffer;
    D3D11_BUFFER_DESC bufferDesc = { };

    bufferDesc.BindFlags = getNativeBindFlags(binds);
    bufferDesc.ByteWidth = widthBytes;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.StructureByteStride = structureByteStride;
    
    getNativeAccessAndUsage(usage, bufferDesc.CPUAccessFlags, bufferDesc.Usage);

    pBuffer->_usage = usage;
    pBuffer->_width = widthBytes;
    pBuffer->_flags = binds;
    pBuffer->_pBackend = this;
 
    HRESULT result = m_pDevice->CreateBuffer(&bufferDesc, 
                                             nullptr,
                                             &pNativeBuffer); 
    DX11ASSERT(result);
    m_resources[pBuffer->getUUID()] = pNativeBuffer;
}


void D3D11Backend::createTexture(Resource** texture, 
                                 ResourceDimension dimension,
                                 ResourceUsage usage, 
                                 ResourceBindFlags binds,
                                 DXGI_FORMAT format, 
                                 U32 width, 
                                 U32 height, 
                                 U32 depth,
                                 U32 structureByteStride,
                                 const TCHAR* debugName)
{
  TextureD3D11* pBuffer = new TextureD3D11(dimension, usage, binds);
  *texture = pBuffer;

  switch (dimension) {
    case RESOURCE_DIMENSION_2D:
      D3D11_TEXTURE2D_DESC textureDesc = {};
      textureDesc.BindFlags = getNativeBindFlags(binds);
      textureDesc.Width = width;
      textureDesc.Height = height;
      textureDesc.Format = format;
      textureDesc.MipLevels = 1;
      textureDesc.ArraySize = 1;
      textureDesc.SampleDesc.Count = 1;
      textureDesc.SampleDesc.Quality = 0;
      getNativeAccessAndUsage(usage, textureDesc.CPUAccessFlags, textureDesc.Usage);
      ID3D11Texture2D* pTex = nullptr;
      HRESULT result =
          m_pDevice->CreateTexture2D(&textureDesc, nullptr, &pTex);
      DX11ASSERT(result);
      
      m_resources[(*texture)->getUUID()] = pTex;
      break;
  }

  pBuffer->_usage = usage;
  pBuffer->_format = format;
  pBuffer->_width = width;
  pBuffer->_height = height;
  pBuffer->_depth = depth;
  pBuffer->_flags = binds;
  pBuffer->_pBackend = this;
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

void D3D11Backend::createRenderTargetView(RenderTargetView** rtv, Resource* buffer, const RenderTargetViewDesc& desc)
{
    ID3D11RenderTargetView* pNativeView = nullptr;
    TargetView* pView = new TargetView();

    ID3D11Resource* pNative = m_resources[buffer->getUUID()];
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = processRenderTargetViewDesc(desc);

    HRESULT result = m_pDevice->CreateRenderTargetView(pNative, &rtvDesc, &pNativeView);
    DX11ASSERT(result);

    m_renderTargetViews[pView->getUUID()] = pNativeView;
    *rtv = pView;
}


void D3D11Backend::createDepthStencilView(DepthStencilView** pDsv, Resource* buffer, const DepthStencilViewDesc& desc)
{
  ID3D11DepthStencilView* pDepthStencil = nullptr;
  TargetView* pView = new TargetView();

  ID3D11Resource* pResource = m_resources[buffer->getUUID()];
  D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = processDepthStencilViewDesc(desc);
  HRESULT result = m_pDevice->CreateDepthStencilView(pResource, &dsvDesc, &pDepthStencil);
  DX11ASSERT(result);
  m_depthStencilViews[pView->getUUID()] = pDepthStencil;
  *pDsv = pView;
}


void D3D11Backend::createDescriptorTable(DescriptorTable** table)
{
  DescriptorTableD3D11* pNative = new DescriptorTableD3D11();
  *table = pNative;
}


void D3D11Backend::createRootSignature(RootSignature** ppRootSig)
{
  RootSignatureD3D11* pRootSig = new RootSignatureD3D11();
  *ppRootSig = pRootSig;
}


void D3D11Backend::createVertexBufferView(VertexBufferView** ppBufferView,
                                          Resource* buffer,
                                          U32 vertexStride,
                                          U32 bufferSzBytes)
{
  VertexBufferViewD3D11* pView = new VertexBufferViewD3D11();
  *ppBufferView = pView;
  pView->_buffer = buffer->getUUID();
  pView->_stride = vertexStride;
  pView->_szBytes = bufferSzBytes;
}


void D3D11Backend::createIndexBufferView(IndexBufferView** ppIndexView,
                                         Resource* pBuffer,
                                         DXGI_FORMAT format,
                                         U32 szBytes)
{
  IndexBufferViewD3D11* pView = new IndexBufferViewD3D11();
  *ppIndexView = pView;
  pView->_buffer = pBuffer->getUUID();
  pView->_format = format;
  pView->_szBytes = szBytes;
}


void D3D11Backend::createGraphicsPipelineState(GraphicsPipeline** ppPipeline,
                                               const GraphicsPipelineInfo* pInfo)
{
    GraphicsPipelineD3D11* pPipeline = new GraphicsPipelineD3D11();
    *ppPipeline = pPipeline;
    
    pPipeline->_topology = getD3D11PrimitiveTopology(pInfo->_topology);
    pPipeline->_numRenderTargets = pInfo->_numRenderTargets;
    pPipeline->_sampleMask = pInfo->_sampleMask;

    if (pInfo->_vertexShader._pByteCode) {
      ID3D11VertexShader* pVertexShader = nullptr;
      DX11ASSERT(m_pDevice->CreateVertexShader(pInfo->_vertexShader._pByteCode, 
                                               pInfo->_vertexShader._szBytes, 
                                               nullptr, 
                                               &pVertexShader));
      m_pVertexShaders[pPipeline->getUUID()] = pVertexShader;
    } 

    if (pInfo->_hullShader._pByteCode) {
      ID3D11HullShader* pHullShader = nullptr;
      DX11ASSERT(m_pDevice->CreateHullShader(pInfo->_hullShader._pByteCode, 
                                             pInfo->_hullShader._szBytes, 
                                             nullptr, 
                                             &pHullShader));
      m_pHullShaders[pPipeline->getUUID()] = pHullShader;
    } 

    if (pInfo->_domainShader._pByteCode) {
      ID3D11DomainShader* pDomainShader = nullptr;
      DX11ASSERT(m_pDevice->CreateDomainShader(pInfo->_domainShader._pByteCode, 
                                               pInfo->_domainShader._szBytes, 
                                               nullptr, 
                                               &pDomainShader));
      m_pDomainShaders[pPipeline->getUUID()] = pDomainShader;
    } 

    if (pInfo->_geometryShader._pByteCode) {
      ID3D11GeometryShader* pGeometryShader = nullptr;
      DX11ASSERT(m_pDevice->CreateGeometryShader(pInfo->_geometryShader._pByteCode,
                                                 pInfo->_geometryShader._szBytes, nullptr,
                                                 &pGeometryShader));
      m_pGeometryShaders[pPipeline->getUUID()] = pGeometryShader;
    } 

    if (pInfo->_pixelShader._pByteCode) {
      ID3D11PixelShader* pPixelShader = nullptr;
      DX11ASSERT(m_pDevice->CreatePixelShader(pInfo->_pixelShader._pByteCode, 
                                              pInfo->_pixelShader._szBytes, 
                                              nullptr, 
                                              &pPixelShader));
      m_pPixelShaders[pPipeline->getUUID()] = pPixelShader;
    }

    {
      ID3D11BlendState* pBlendState = nullptr;
      D3D11_BLEND_DESC blendDesc = { };
      blendDesc.AlphaToCoverageEnable = pInfo->_blendState._alphaToCoverageEnable;
      blendDesc.IndependentBlendEnable = pInfo->_blendState._independentBlendEnable;
      for (U32 i = 0; i < 8; ++i) {
        blendDesc.RenderTarget[i].BlendEnable = pInfo->_blendState._renderTargets[i]._blendEnable;
        blendDesc.RenderTarget[i].BlendOp  = getD3D11BlendOp(pInfo->_blendState._renderTargets[i]._blendOp);
        blendDesc.RenderTarget[i].BlendOpAlpha = getD3D11BlendOp(pInfo->_blendState._renderTargets[i]._blendOpAlpha);
        blendDesc.RenderTarget[i].DestBlend = getD3D11Blend(pInfo->_blendState._renderTargets[i]._dstBlend);
        blendDesc.RenderTarget[i].DestBlendAlpha = getD3D11Blend(pInfo->_blendState._renderTargets[i]._dstBlendAlpha);
        blendDesc.RenderTarget[i].RenderTargetWriteMask = pInfo->_blendState._renderTargets[i]._renderTargetWriteMask;
        blendDesc.RenderTarget[i].SrcBlend = getD3D11Blend(pInfo->_blendState._renderTargets[i]._srcBlend);
        blendDesc.RenderTarget[i].SrcBlendAlpha = getD3D11Blend(pInfo->_blendState._renderTargets[i]._srcBlendAlpha);
      }

      DX11ASSERT(m_pDevice->CreateBlendState(&blendDesc, &pBlendState));
      m_pBlendStates[pPipeline->getUUID()] = pBlendState;
    }

    {
      ID3D11DepthStencilState* pDepthStencilState = nullptr;
      D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { };
      depthStencilDesc.BackFace.StencilDepthFailOp = getD3D11StencilOp(pInfo->_depthStencilState._backFace._stencilDepthFailOp);
      depthStencilDesc.BackFace.StencilFailOp = getD3D11StencilOp(pInfo->_depthStencilState._backFace._stencilFailOp);
      depthStencilDesc.BackFace.StencilFunc = getD3D11ComparisonFunc(pInfo->_depthStencilState._backFace._stencilFunc);
      depthStencilDesc.BackFace.StencilPassOp = getD3D11StencilOp(pInfo->_depthStencilState._backFace._stencilPassOp);

      depthStencilDesc.DepthEnable = pInfo->_depthStencilState._depthEnable;
      depthStencilDesc.DepthFunc = getD3D11ComparisonFunc(pInfo->_depthStencilState._depthFunc);
      depthStencilDesc.DepthWriteMask = getD3D11WriteMask(pInfo->_depthStencilState._depthWriteMask);
      depthStencilDesc.FrontFace.StencilDepthFailOp = getD3D11StencilOp(pInfo->_depthStencilState._frontFace._stencilDepthFailOp);
      depthStencilDesc.FrontFace.StencilFailOp = getD3D11StencilOp(pInfo->_depthStencilState._frontFace._stencilFailOp);
      depthStencilDesc.FrontFace.StencilFunc = getD3D11ComparisonFunc(pInfo->_depthStencilState._frontFace._stencilFunc);
      depthStencilDesc.FrontFace.StencilPassOp = getD3D11StencilOp(pInfo->_depthStencilState._frontFace._stencilPassOp);

      depthStencilDesc.StencilEnable = pInfo->_depthStencilState._stencilEnable;
      depthStencilDesc.StencilReadMask = pInfo->_depthStencilState._stencilReadMask;
      depthStencilDesc.StencilWriteMask = pInfo->_depthStencilState._stencilWriteMask;
      DX11ASSERT(m_pDevice->CreateDepthStencilState(&depthStencilDesc, &pDepthStencilState));
      m_pDepthStencilStates[pPipeline->getUUID()] = pDepthStencilState;
    }

    {
      ID3D11RasterizerState* rasterState = { };
      D3D11_RASTERIZER_DESC desc = { };
      desc.AntialiasedLineEnable = pInfo->_rasterizationState._antialiasedLinesEnable;
      desc.CullMode = getD3D11CullMode(pInfo->_rasterizationState._cullMode);
      desc.DepthBias = pInfo->_rasterizationState._depthBias;
      desc.DepthBiasClamp = pInfo->_rasterizationState._depthBiasClamp;
      desc.DepthClipEnable = pInfo->_rasterizationState._depthClipEnable;
      desc.FillMode = getD3D11FillMode(pInfo->_rasterizationState._fillMode);
      desc.FrontCounterClockwise = pInfo->_rasterizationState._frontCounterClockwise;
      desc.MultisampleEnable = pInfo->_rasterizationState._multisampleEnable;
      desc.ScissorEnable = true;
      desc.SlopeScaledDepthBias = pInfo->_rasterizationState._slopedScaledDepthBias;
      DX11ASSERT(m_pDevice->CreateRasterizerState(&desc, &rasterState));
      m_pRasterizationStates[pPipeline->getUUID()] = rasterState;
    }

    {
      std::vector<D3D11_INPUT_ELEMENT_DESC> elementDescs(pInfo->_inputLayout._elementCount);
      ID3D11InputLayout* pInputLayout = nullptr;
      for (U32 i = 0; i < elementDescs.size(); ++i) {
        elementDescs[i].AlignedByteOffset = pInfo->_inputLayout._pInputElements[i]._alignedByteOffset;
        elementDescs[i].Format = pInfo->_inputLayout._pInputElements[i]._format;
        elementDescs[i].InputSlot = pInfo->_inputLayout._pInputElements[i]._inputSlot;
        elementDescs[i].InputSlotClass = getD3D11InputClassification(pInfo->_inputLayout._pInputElements[i]._classification);
        elementDescs[i].InstanceDataStepRate = pInfo->_inputLayout._pInputElements[i]._instanceDataStepRate;
        elementDescs[i].SemanticIndex = pInfo->_inputLayout._pInputElements[i]._semanticIndex;
        elementDescs[i].SemanticName = pInfo->_inputLayout._pInputElements[i]._semanticName;
      }

      DX11ASSERT(m_pDevice->CreateInputLayout(elementDescs.data(), 
                                              elementDescs.size(), 
                                              pInfo->_vertexShader._pByteCode, 
                                              pInfo->_vertexShader._szBytes, 
                                              &pInputLayout));
      m_pInputLayouts[pPipeline->getUUID()] = pInputLayout;
    }

#if 0
    if (pInfo->_com{
      ID3D11ComputeShader* pComputeShader = nullptr;
      DX11ASSERT(m_pDevice->CreateComputeShader(pBytecode->_pByteCode,
                                                pBytecode->_szBytes, nullptr,
                                                &pComputeShader));
      m_pComputeShaders[(*ppShader)->getUUID()] = pComputeShader;
    }
#endif 
}


void D3D11Backend::createRenderPass(RenderPass** pRenderPass, 
                                    U32 rtvSize,
                                    B32 hasDepthStencil)
{
  *pRenderPass = new RenderPassD3D11();
}


void D3D11Backend::destroyRenderPass(RenderPass* pRenderPass)
{
  delete pRenderPass;
}
} // gfx