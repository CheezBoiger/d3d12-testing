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


    void setUnorderedAccessViews(UnorderedAccessView** uavs, U32 uavCount) override { 
        _unorderedAccessViews.resize(uavCount);
        for (U32 i = 0; i < uavCount; ++i) {
            _unorderedAccessViews[i] = uavs[i];
        }
    }


    void setShaderResourceViews(ShaderResourceView** buffers, U32 viewCount) override {
        _shaderResourceViews.resize(viewCount);
        for (U32 i = 0; i < viewCount; ++i) {
            _shaderResourceViews[i] = buffers[i];
        }
    }


    void initialize(DescriptorTableType type, U32 totalCount) override { 
        createDescriptorHeapForTable(type, totalCount);
    }

    void update(DescriptorTableFlags flags) override {
        updateDescriptorHeapTable(flags);
    }

private:

    void createDescriptorHeapForTable(DescriptorTableType type, U32 totalCount) {
        m_type = type;
        ID3D12DescriptorHeap* pHeap = nullptr;
        if (m_type == DESCRIPTOR_TABLE_SRV_UAV_CBV) {
            if (getBackendD3D12()->getDescriptorHeap(getUUID())) {
                getBackendD3D12()->getDescriptorHeap(getUUID())->Release();
            }
            U32 descriptorCount = totalCount;
            D3D12_DESCRIPTOR_HEAP_DESC desc = { };
            desc.NumDescriptors = descriptorCount;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NodeMask = 0;
            DX12ASSERT(getBackendD3D12()->getDevice()->CreateDescriptorHeap(&desc, 
                                                                    __uuidof(ID3D12DescriptorHeap), 
                                                                    (void**)&pHeap));
    
            m_descriptorOffset = pHeap->GetCPUDescriptorHandleForHeapStart();
            getBackendD3D12()->setDescriptorHeap(getUUID(), pHeap);
        } else if (m_type == DESCRIPTOR_TABLE_SAMPLER) {
            if (getBackendD3D12()->getDescriptorHeap(getUUID())) {
                getBackendD3D12()->getDescriptorHeap(getUUID())->Release();
            }
            D3D12_DESCRIPTOR_HEAP_DESC samplerDesc = { };
            samplerDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            samplerDesc.NodeMask = 0;
            samplerDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            samplerDesc.NumDescriptors = totalCount;
            DX12ASSERT(getBackendD3D12()->getDevice()->CreateDescriptorHeap(&samplerDesc, 
                                                                            __uuidof(ID3D12DescriptorHeap), 
                                                                            (void**)&pHeap));
            m_descriptorOffset = pHeap->GetCPUDescriptorHandleForHeapStart();
            U32 incSize = getBackendD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            getBackendD3D12()->setDescriptorHeap(getUUID(), pHeap);
        }
    }
  
  void updateDescriptorHeapTable(DescriptorTableFlags flags) {
    if (!_constantBuffers.empty() || !_shaderResourceViews.empty() || !_unorderedAccessViews.empty()) {
        ID3D12DescriptorHeap* pHeap = getBackendD3D12()->getDescriptorHeap(getUUID());
        if (flags & DESCRIPTOR_TABLE_FLAG_RESET) {
            m_descriptorOffset = pHeap->GetCPUDescriptorHandleForHeapStart();
        }
        U32 incSize = 
          getBackendD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (U32 i = 0; i < _constantBuffers.size(); ++i) {
            BufferD3D12* pConstBuffer = static_cast<BufferD3D12*>(_constantBuffers[i]);
            ID3D12Resource* pResource = getBackendD3D12()->getResource(pConstBuffer->getUUID());
            D3D12_CONSTANT_BUFFER_VIEW_DESC constDesc = { };
            constDesc.BufferLocation = pResource->GetGPUVirtualAddress();
            constDesc.SizeInBytes = (pResource->GetDesc().Width + 255) & ~255;
            getBackendD3D12()->getDevice()->CreateConstantBufferView(&constDesc, m_descriptorOffset);
            m_descriptorOffset.ptr += incSize;
        } 

        for (U32 i = 0; i < _shaderResourceViews.size(); ++i) {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = { };
            ViewHandleD3D12* view = static_cast<ViewHandleD3D12*>(_shaderResourceViews[i]);
            D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = getBackendD3D12()->getViewHandle(view->getUUID(), 0);
            UINT sz = 1;
            getBackendD3D12()->getDevice()->CopyDescriptors(1, &m_descriptorOffset, &sz, 1, &srcHandle, &sz, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            m_descriptorOffset.ptr += incSize;
        }
    
        for (U32 i = 0; i < _unorderedAccessViews.size(); ++i) {
          D3D12_UNORDERED_ACCESS_VIEW_DESC desc = { };
          m_descriptorOffset.ptr += incSize;
        }
    }


    if (_samplers.empty()) {
        return;
    }

    U32 incSize = getBackendD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    for (U32 i = 0; i < _samplers.size(); ++i) {
        UINT sz = 1;
        D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = getBackendD3D12()->getSamplerDescriptorHandle(_samplers[i]->getUUID());
        getBackendD3D12()->getDevice()->CopyDescriptors(1, &m_descriptorOffset, &sz, 1, &srcHandle, &sz, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);   
        m_descriptorOffset.ptr += incSize;
    }
  }

public:

    std::vector<Resource*> _constantBuffers;
    std::vector<ShaderResourceView*> _shaderResourceViews;
    std::vector<UnorderedAccessView*> _unorderedAccessViews;
    std::vector<Sampler*> _samplers;
    std::vector<SamplerDesc> _staticSamplers;
  
private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorOffset;
    DescriptorTableType m_type;
};
} // gfx