#pragma once



#include "../Renderer.h"
#include "D3D12Backend.h"

namespace gfx {

D3D12_ROOT_SIGNATURE_FLAGS getNativeRootSignatureFlags(ShaderVisibilityFlags flags) {
  D3D12_ROOT_SIGNATURE_FLAGS v = D3D12_ROOT_SIGNATURE_FLAG_NONE;
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


class RootSignatureD3D12 : public RootSignature
{
public:
  RootSignatureD3D12(){ }

  void initialize(ShaderVisibilityFlags visibleFlags, 
                  PipelineLayout* pLayouts, 
                  U32 numLayouts) override {
    static D3D12_ROOT_PARAMETER kParameters[128];
    static D3D12_STATIC_SAMPLER_DESC kStaticSamplerDescs[128];
    U32 parameterCount = 0;

    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> kDescriptorRanges(numLayouts);

    ID3D12RootSignature* pRootSig = nullptr;
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};

    for (U32 i = 0; i < numLayouts; ++i) {
      D3D12_ROOT_DESCRIPTOR_TABLE rootTable = {};
      U32 numCBV_SRV_UAV_Descriptors = 0;

      if (pLayouts[i].numConstantBuffers > 0) {
        D3D12_DESCRIPTOR_RANGE range = {};
        range.NumDescriptors = static_cast<U32>(pLayouts[i].numConstantBuffers);
        range.BaseShaderRegister = 0;
        range.OffsetInDescriptorsFromTableStart =
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        range.RegisterSpace = 0;
        kDescriptorRanges[i].push_back(range);
      }

      if (pLayouts[i].numShaderResourceViews) {
        D3D12_DESCRIPTOR_RANGE range = {};
        range.NumDescriptors = static_cast<U32>(pLayouts[i].numShaderResourceViews);
        range.OffsetInDescriptorsFromTableStart =
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        range.BaseShaderRegister = 0;
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.RegisterSpace = 0;
        kDescriptorRanges[i].push_back(range);
      }

      if (pLayouts[i].numUnorderedAcessViews) {
        D3D12_DESCRIPTOR_RANGE range = {};
        range.NumDescriptors = static_cast<U32>(pLayouts[i].numUnorderedAcessViews);
        range.OffsetInDescriptorsFromTableStart =
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        range.BaseShaderRegister = 0;
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        range.RegisterSpace = 0;
        kDescriptorRanges[i].push_back(range);
      }

      if (pLayouts[i].numSamplers) {
        D3D12_DESCRIPTOR_RANGE range = { };
        range.NumDescriptors = pLayouts[i].numSamplers;
        range.OffsetInDescriptorsFromTableStart = 
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        range.BaseShaderRegister = 0;
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        range.RegisterSpace = 0;
        kDescriptorRanges[i].push_back(range);
      }

      rootTable.NumDescriptorRanges = static_cast<U32>(kDescriptorRanges[i].size());
      rootTable.pDescriptorRanges = kDescriptorRanges[i].data();

      D3D12_ROOT_PARAMETER rootParam = {};
      rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
      rootParam.DescriptorTable = rootTable;
      rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
      kParameters[parameterCount++] = rootParam;
    }

      rootSigDesc.NumParameters = parameterCount;
      rootSigDesc.pParameters = kParameters;
      rootSigDesc.NumStaticSamplers = 0;
      rootSigDesc.pStaticSamplers = kStaticSamplerDescs;
      rootSigDesc.Flags = getNativeRootSignatureFlags(visibleFlags);

      ID3DBlob* pRootSigBlob = nullptr;
      DX12ASSERT(D3D12SerializeRootSignature(&rootSigDesc, 
                                             D3D_ROOT_SIGNATURE_VERSION_1_0, 
                                             &pRootSigBlob, 
                                             nullptr));
      DX12ASSERT(getBackendD3D12()->getDevice()->CreateRootSignature(0, 
                                                                     pRootSigBlob->GetBufferPointer(), 
                                                                     pRootSigBlob->GetBufferSize(),
                                                                     __uuidof(ID3D12RootSignature), 
                                                                     (void**)&pRootSig));
      getBackendD3D12()->setRootSignature(getUUID(), pRootSig);

  }

};

} // gfx