
#pragma once


#include "CommonsD3D12.h"

#define KB_1 (1024)
#define MB_1 (1024 * 1024)

namespace gfx {


/*
    One of the most powerful, yet overwhelming, features of D3D12 is the ability 
    to manage your own memory. Allocation of large memory heaps is essential to speeding up
    creation of game objects, and effects. With a large allocation, developers can then figure out
    how to handle this giant memory region, suballocating from it to placed resources to be used by
    constant buffers, vertex/index buffers, shader resource views, image, etc...

    This is not for the faintest of heart, however, as CPU/GPU memory management can be a huge turn off
    from most developers. This is why libraries exists from hardware vendors, to handle these cases.
*/
class MemoryAllocatorD3D12 
{
public:
    void initialize(ID3D12Device* pDevice, size_t regionSzBytes) {
        //D3D12_RESOURCE_ALLOCATION_INFO allocRules = pDevice->GetResourceAllocationInfo(0, )

        // 64kb alignment granularity for all GPUs, this means page sizes range of this range,
        // and must either contain multiple textures, one single texture, multiple buffers,
        // or one single buffer. Take into account aliasing if textures and buffers overlap.
        // For now, we are allocating 512 megabytes for GPU bound work.
        D3D12_HEAP_DESC heapDesc = { };

        heapDesc.SizeInBytes = 512 * MB_1;

        heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_CUSTOM;
        heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
        heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
        heapDesc.Properties.CreationNodeMask = 0;
        pDevice->CreateHeap(&heapDesc, __uuidof(ID3D12Heap), (void**)&m_pArena);
    }


    ID3D12Resource* allocate(const D3D12_RESOURCE_DESC& desc) { return nullptr; }

    void free(ID3D12Resource* pResource) { }
private:
    ID3D12Heap* m_pArena;
    U32 m_garbageIndex;
};
} // gfx