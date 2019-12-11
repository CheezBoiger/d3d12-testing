#pragma once



#include "../BackendRenderer.h"
#include "D3D12Backend.h"

namespace gfx {

D3D12_ROOT_SIGNATURE_FLAGS getNativeRootSignatureFlags(ShaderVisibilityFlags flags) {
  D3D12_ROOT_SIGNATURE_FLAGS v = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
  if (!(flags & SHADER_VISIBILITY_DOMAIN)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
  }
  if (!(flags & SHADER_VISIBILITY_HULL)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
  }
  if (!(flags & SHADER_VISIBILITY_VERTEX)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
  }
  if (!(flags & SHADER_VISIBILITY_PIXEL)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
  }
  if (!(flags & SHADER_VISIBILITY_GEOMETRY)) {
    v |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
  }
  return v;
}


D3D12_ROOT_PARAMETER_TYPE getRootParameterType(PipelineLayoutType type)
{
  switch (type) {
    case PIPELINE_LAYOUT_TYPE_CBV: return D3D12_ROOT_PARAMETER_TYPE_CBV;
    case PIPELINE_LAYOUT_TYPE_CONSTANTS: return D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    case PIPELINE_LAYOUT_TYPE_SRV: return D3D12_ROOT_PARAMETER_TYPE_SRV;    
    case PIPELINE_LAYOUT_TYPE_UAV: return D3D12_ROOT_PARAMETER_TYPE_UAV;
    default:
    case PIPELINE_LAYOUT_TYPE_DESCRIPTOR_TABLE: return D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  }
}


class RootSignatureD3D12 : public RootSignature
{
public:
  RootSignatureD3D12(){ }

  void initialize(ShaderVisibilityFlags visibleFlags, 
                  PipelineLayout* pLayouts, 
                  U32 numLayouts,
                    StaticSamplerDesc* pStaticSamplers = nullptr,
                    U32 staticSamplerCount = 0) override {
    static D3D12_ROOT_PARAMETER kParameters[64];
    static D3D12_STATIC_SAMPLER_DESC kStaticSamplerDescs[32];
    U32 parameterCount = 0;
    U32 staticSamplerC = 0;

    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> kDescriptorRanges(numLayouts);

    ID3D12RootSignature* pRootSig = nullptr;
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    U32 cbvRegister = 0;
    U32 srvRegister = 0;
    U32 uavRegister = 0;
    U32 samplerRegister = 0;

    for (U32 i = 0; i < numLayouts; ++i) {
      D3D12_ROOT_PARAMETER rootParam = {};
      rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
      rootParam.ParameterType = getRootParameterType(pLayouts[i]._type);

      switch (rootParam.ParameterType) {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: {
          D3D12_ROOT_DESCRIPTOR_TABLE rootTable = {};
          U32 numCBV_SRV_UAV_Descriptors = 0;
 
          if (pLayouts[i]._numConstantBuffers > 0) {
            D3D12_DESCRIPTOR_RANGE range = {};
            range.NumDescriptors = static_cast<U32>(pLayouts[i]._numConstantBuffers);
            range.BaseShaderRegister = cbvRegister++;
            range.OffsetInDescriptorsFromTableStart =
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            range.RegisterSpace = 0;
            kDescriptorRanges[i].push_back(range);
          }

          if (pLayouts[i]._numShaderResourceViews) {
            D3D12_DESCRIPTOR_RANGE range = {};
            range.NumDescriptors = static_cast<U32>(pLayouts[i]._numShaderResourceViews);
            range.OffsetInDescriptorsFromTableStart =
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            range.BaseShaderRegister = srvRegister++;
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.RegisterSpace = 0;
            kDescriptorRanges[i].push_back(range);
          }

          if (pLayouts[i]._numUnorderedAcessViews) {
            D3D12_DESCRIPTOR_RANGE range = {};
            range.NumDescriptors = static_cast<U32>(pLayouts[i]._numUnorderedAcessViews);
            range.OffsetInDescriptorsFromTableStart =
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            range.BaseShaderRegister = uavRegister++;
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            range.RegisterSpace = 0;
            kDescriptorRanges[i].push_back(range);
          }

          if (pLayouts[i]._numSamplers) {
            D3D12_DESCRIPTOR_RANGE range = { };
            range.NumDescriptors = pLayouts[i]._numSamplers;
            range.OffsetInDescriptorsFromTableStart = 
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            range.BaseShaderRegister = samplerRegister++;
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            range.RegisterSpace = 0;
            kDescriptorRanges[i].push_back(range);
          }

          rootTable.NumDescriptorRanges = static_cast<U32>(kDescriptorRanges[i].size());
          rootTable.pDescriptorRanges = kDescriptorRanges[i].data();
          rootParam.DescriptorTable = rootTable;
          break;
        }
        case D3D12_ROOT_PARAMETER_TYPE_CBV: {
          rootParam.Descriptor.RegisterSpace = 0;
          rootParam.Descriptor.ShaderRegister = cbvRegister++;
          break;
        }
        case D3D12_ROOT_PARAMETER_TYPE_SRV: {
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParam.Descriptor.RegisterSpace = 0;
            rootParam.Descriptor.ShaderRegister = srvRegister++;
            break;
        }
        case D3D12_ROOT_PARAMETER_TYPE_UAV: {
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            rootParam.Descriptor.RegisterSpace = 0;
            rootParam.Descriptor.ShaderRegister = uavRegister++;
        } break;
      }

      kParameters[parameterCount++] = rootParam;
    }

    if (staticSamplerCount > 0) {
        for (U32 i = 0; i < staticSamplerCount; ++i) {
            kStaticSamplerDescs[i] = { };
            kStaticSamplerDescs[i].AddressU = getNativeTextureAddress(pStaticSamplers[i]._addressU);   
            kStaticSamplerDescs[i].AddressV = getNativeTextureAddress(pStaticSamplers[i]._addressV);
            kStaticSamplerDescs[i].AddressW = getNativeTextureAddress(pStaticSamplers[i]._addressW);
            kStaticSamplerDescs[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            kStaticSamplerDescs[i].ComparisonFunc = getComparisonFunc(pStaticSamplers[i]._comparisonFunc);
            kStaticSamplerDescs[i].Filter = getNativeFilter(pStaticSamplers[i]._filter);
            kStaticSamplerDescs[i].MaxAnisotropy = pStaticSamplers[i]._maxAnisotropy;
            kStaticSamplerDescs[i].MaxLOD = pStaticSamplers[i]._maxLod;
            kStaticSamplerDescs[i].MinLOD = pStaticSamplers[i]._minLod;
            kStaticSamplerDescs[i].MipLODBias = pStaticSamplers[i]._mipLodBias;
            kStaticSamplerDescs[i].RegisterSpace = pStaticSamplers[i]._registerSpace;
            kStaticSamplerDescs[i].ShaderRegister = pStaticSamplers[i]._shaderRegister;
            kStaticSamplerDescs[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        }

        staticSamplerC = staticSamplerCount;
    }

      rootSigDesc.NumParameters = parameterCount;
      rootSigDesc.pParameters = kParameters;
      rootSigDesc.NumStaticSamplers = staticSamplerC;
      rootSigDesc.pStaticSamplers = kStaticSamplerDescs;
      rootSigDesc.Flags = getNativeRootSignatureFlags(visibleFlags);

      ID3DBlob* pRootSigBlob = nullptr;
      ID3DBlob* pErrorBlob = nullptr;
      HRESULT result = D3D12SerializeRootSignature(&rootSigDesc, 
                                             D3D_ROOT_SIGNATURE_VERSION_1_0, 
                                             &pRootSigBlob, 
                                             &pErrorBlob);
      if (pErrorBlob) {
        char* pp = reinterpret_cast<char*>(pErrorBlob->GetBufferPointer());
        DEBUG("Error: %s", pp);
        pErrorBlob->Release();
      }

      DX12ASSERT(result);
      DX12ASSERT(getBackendD3D12()->getDevice()->CreateRootSignature(0, 
                                                                     pRootSigBlob->GetBufferPointer(), 
                                                                     pRootSigBlob->GetBufferSize(),
                                                                     __uuidof(ID3D12RootSignature), 
                                                                     (void**)&pRootSig));
      getBackendD3D12()->setRootSignature(getUUID(), pRootSig);

  }

};

} // gfx