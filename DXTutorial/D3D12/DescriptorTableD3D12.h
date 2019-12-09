//
#pragma once

#include "../BackendRenderer.h"
#include "D3D12Backend.h"

#include <vector>

namespace gfx {


struct DescriptorTableD3D12 : public DescriptorTable {
  DescriptorTableD3D12() { }

  void setConstantBuffers(Resource** buffers, U32 bufferCount) override {
    _constantBuffers.resize(bufferCount);
    for (U32 i = 0; i < bufferCount; ++i) {
      _constantBuffers[i] = buffers[i];
    }
  }

    void setSamplers(Sampler** sampler, U32 samplerCount) override {
        _samplers.resize(samplerCount);
        for (U32 i = 0; i < samplerCount; ++i) {
            _samplers[i] = sampler[i];   
        }      
    }

  void finalize() override { 
    createDescriptorHeapForTable();
  }

private:
  
  void createDescriptorHeapForTable() {

    ID3D12DescriptorHeap* pHeap = nullptr;
    if (!_constantBuffers.empty() || !_shaderResourceViews.empty() || !_unorderedAccessViews.empty()) {
        U32 descriptorCount = static_cast<U32>(_constantBuffers.size() 
                                             + _shaderResourceViews.size() 
                                             + _unorderedAccessViews.size());
        D3D12_DESCRIPTOR_HEAP_DESC desc = { };
        desc.NumDescriptors = descriptorCount;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NodeMask = 0;
        DX12ASSERT(getBackendD3D12()->getDevice()->CreateDescriptorHeap(&desc, 
                                                                __uuidof(ID3D12DescriptorHeap), 
                                                                (void**)&pHeap));
        U32 incSize = 
          getBackendD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
        D3D12_CPU_DESCRIPTOR_HANDLE offset = pHeap->GetCPUDescriptorHandleForHeapStart();
        for (U32 i = 0; i < _constantBuffers.size(); ++i) {
            BufferD3D12* pConstBuffer = static_cast<BufferD3D12*>(_constantBuffers[i]);
            ID3D12Resource* pResource = getBackendD3D12()->getResource(pConstBuffer->getUUID());
            D3D12_CONSTANT_BUFFER_VIEW_DESC constDesc = { };
            constDesc.BufferLocation = pResource->GetGPUVirtualAddress();
            constDesc.SizeInBytes = (pResource->GetDesc().Width + 255) & ~255;
            getBackendD3D12()->getDevice()->CreateConstantBufferView(&constDesc, offset);
            offset.ptr += incSize;
        }

        for (U32 i = 0; i < _shaderResourceViews.size(); ++i) {
          D3D12_SHADER_RESOURCE_VIEW_DESC desc = { };
          offset.ptr += incSize;
        }
    
        for (U32 i = 0; i < _unorderedAccessViews.size(); ++i) {
          D3D12_UNORDERED_ACCESS_VIEW_DESC desc = { };
          offset.ptr += incSize;
        }

        getBackendD3D12()->setDescriptorHeap(getUUID(), pHeap);
    }

    if (_samplers.empty()) {
        return;
    }

    D3D12_DESCRIPTOR_HEAP_DESC samplerDesc = { };
    samplerDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    samplerDesc.NodeMask = 0;
    samplerDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerDesc.NumDescriptors = _samplers.size();
    DX12ASSERT(getBackendD3D12()->getDevice()->CreateDescriptorHeap(&samplerDesc, __uuidof(ID3D12DescriptorHeap), (void**)&pHeap));
    D3D12_CPU_DESCRIPTOR_HANDLE offset = pHeap->GetCPUDescriptorHandleForHeapStart();
    
    U32 incSize = getBackendD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    for (U32 i = 0; i < _samplers.size(); ++i) {
        UINT sz = 1;
        D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = getBackendD3D12()->getSamplerDescriptorHandle(_samplers[i]->getUUID());
        getBackendD3D12()->getDevice()->CopyDescriptors(1, &offset, &sz, 1, &srcHandle, &sz, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);   
        offset.ptr += incSize;
    }
    
    getBackendD3D12()->setDescriptorHeap(getUUID(), pHeap);
  }

public:

  std::vector<Resource*> _constantBuffers;
  std::vector<Resource*> _shaderResourceViews;
  std::vector<Resource*> _unorderedAccessViews;
  std::vector<Sampler*> _samplers;
  
};
} // gfx