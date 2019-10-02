//
#pragma once

#include "../Renderer.h"
#include "D3D12Backend.h"

#include <vector>

namespace gfx {


struct DescriptorTableD3D12 : public DescriptorTable {
  DescriptorTableD3D12(D3D12Backend* pBackend) 
    : _pBackend(pBackend) { }

  void setConstantBuffers(Resource** buffers, U32 bufferCount) override {
    _constantBuffers.resize(bufferCount);
    for (U32 i = 0; i < bufferCount; ++i) {
      _constantBuffers[i] = buffers[i];
    }
  }

  void finalize(ShaderVisibilityFlags visibleFlags) override { 
    static D3D12_DESCRIPTOR_RANGE kRanges[128];
    static D3D12_STATIC_SAMPLER_DESC kStaticSamplerDescs[128];
    ID3D12RootSignature* pRootSig = nullptr;
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = { };
    D3D12_ROOT_DESCRIPTOR_TABLE rootTable = { };
    U32 parameterCount = 0;
    U32 numCBV_SRV_UAV_Descriptors = 0;

    if (!_constantBuffers.empty()) {
      D3D12_DESCRIPTOR_RANGE range = {};
      range.NumDescriptors = static_cast<U32>(_constantBuffers.size());
      range.BaseShaderRegister = 0;
      range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
      range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
      range.RegisterSpace = 0;
      kRanges[parameterCount] = range;
      parameterCount += 1;
    }

    if (!_shaderResourceViews.empty()) {
      D3D12_DESCRIPTOR_RANGE range = { };
      range.NumDescriptors = static_cast<U32>(_shaderResourceViews.size());
      range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
      range.BaseShaderRegister = 0; 
      range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      range.RegisterSpace = 0;
      kRanges[parameterCount] = range;
      parameterCount += 1;
    }

    if (!_unorderedAccessViews.empty()) {
      D3D12_DESCRIPTOR_RANGE range = {};
      range.NumDescriptors = static_cast<U32>(_unorderedAccessViews.size());
      range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
      range.BaseShaderRegister = 0;
      range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      range.RegisterSpace = 0;
      kRanges[parameterCount] = range;
      parameterCount += 1;
    }

    rootTable.NumDescriptorRanges = parameterCount;
    rootTable.pDescriptorRanges = kRanges;

    D3D12_ROOT_PARAMETER rootParam = { };
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam.DescriptorTable = rootTable;    
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    
    rootSigDesc.NumParameters = 1;
    rootSigDesc.pParameters = &rootParam;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = kStaticSamplerDescs;

    ID3DBlob* pRootSigBlob = nullptr;
    DX12ASSERT(D3D12SerializeRootSignature(&rootSigDesc, 
                                D3D_ROOT_SIGNATURE_VERSION_1_0,
                                &pRootSigBlob,
                                nullptr));
    DX12ASSERT(_pBackend->getDevice()->CreateRootSignature(0, 
                                                           pRootSigBlob->GetBufferPointer(), 
                                                           pRootSigBlob->GetBufferSize(), 
                                                           __uuidof(ID3D12RootSignature), 
                                                           (void**)&pRootSig));
    _pBackend->setRootSignature(getUUID(), pRootSig);

    createDescriptorHeapForTable();
  }

private:
  
  void createDescriptorHeapForTable() {
    ID3D12DescriptorHeap* pHeap = nullptr;
    U32 descriptorCount = static_cast<U32>(_constantBuffers.size() 
                                         + _shaderResourceViews.size() 
                                         + _unorderedAccessViews.size());
    D3D12_DESCRIPTOR_HEAP_DESC desc = { };
    desc.NumDescriptors = descriptorCount;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NodeMask = 0;
    DX12ASSERT(_pBackend->getDevice()->CreateDescriptorHeap(&desc, 
                                                            __uuidof(ID3D12DescriptorHeap), 
                                                            (void**)&pHeap));
    U32 incSize = 
      _pBackend->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    D3D12_CPU_DESCRIPTOR_HANDLE offset = pHeap->GetCPUDescriptorHandleForHeapStart();
    for (U32 i = 0; i < _constantBuffers.size(); ++i) {
        BufferD3D12* pConstBuffer = static_cast<BufferD3D12*>(_constantBuffers[i]);
        ID3D12Resource* pResource = _pBackend->getResource(pConstBuffer->getUUID());
        D3D12_CONSTANT_BUFFER_VIEW_DESC constDesc = { };
        constDesc.BufferLocation = pResource->GetGPUVirtualAddress();
        constDesc.SizeInBytes = (pResource->GetDesc().Width + 255) & ~255;
        _pBackend->getDevice()->CreateConstantBufferView(&constDesc, offset);
        offset.ptr += incSize;
    }

    for (U32 i = 0; i < _shaderResourceViews.size(); ++i) {
      offset.ptr += incSize;
    }
    
    for (U32 i = 0; i < _unorderedAccessViews.size(); ++i) {
      offset.ptr += incSize;
    }

    _pBackend->setDescriptorHeap(getUUID(), pHeap);
  }

public:

  D3D12Backend* _pBackend;
  std::vector<Resource*> _constantBuffers;
  std::vector<Resource*> _shaderResourceViews;
  std::vector<Resource*> _unorderedAccessViews;
  std::vector<Resource*> _samplers;
  
};
} // gfx