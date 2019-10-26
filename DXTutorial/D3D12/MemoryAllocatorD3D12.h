/*
    One of the most powerful, yet overwhelming, features of D3D12 is the ability
    to manage your own memory. Allocation of large memory heaps is essential to speeding up
    creation of game objects, and effects. With a large allocation, developers can then figure out
    how to handle this giant memory region, suballocating from it to placed resources to be used by
    constant buffers, vertex/index buffers, shader resource views, image, etc...

    Concept derives from Region Based Memory Allocation, or otherwise known as Memory Arena management,
    in order to devise your own way of handling memory allocations. This can be avoided, however, by
    simply creating committed allocations that allow the gpu to dedicate a memory block on the gpu
    (similar to how d3d11 handled memory managment.)

    This is not for the faintest of heart, however, as CPU/GPU memory management can be a huge turn off
    from most developers. This is why libraries exists from hardware vendors, to handle these cases.
*/
#pragma once


#include "CommonsD3D12.h"
#include "../BackendRenderer.h"

#include <vector>

#define KB_1 (1024)
#define MB_1 (1024 * 1024)
#define GB_1 (1024 * 1024 * 1024)
#define ALIGN(p, alignment) (( p ) + (( alignment ) - 1)) & (~(( alignment ) - 1))

namespace gfx {

struct MemoryResource
{
    ID3D12Resource* _pResource;
    size_t _poolId;
    size_t _blockId;
    size_t _offset;
};

// Abstract memory pool class, which holds the native handled memory heap. Depending on the heap type,
// determines whether this heap is located in host (cpu memory) or device (gpu memory). 
class MemoryPool
{
public:

    void initialize
        (
            // Device.
            ID3D12Device* pDevice,
            // max region size in bytes. 
            size_t regionSzBytes
        ) 
    {
        //D3D12_RESOURCE_ALLOCATION_INFO allocRules = pDevice->GetResourceAllocationInfo(0, )

        // 64kb alignment granularity for all GPUs, this means page sizes of this range,
        // which must either contain multiple textures, one single texture, multiple buffers,
        // or one single buffer. Take into account aliasing if textures and buffers overlap.
        // 

        // Fails on initiailze, return without anything :(
        if (!onInitialize(regionSzBytes)) return;

        D3D12_HEAP_DESC heapDesc = { };

        heapDesc.SizeInBytes = regionSzBytes;

        heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
        heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
        heapDesc.Properties.CreationNodeMask = 0;
        
        pDevice->CreateHeap(&heapDesc, __uuidof(ID3D12Heap), (void**)&m_pArena);
    }

    void cleanUp
        (
            // Device.
            ID3D12Device* pDevice
        ) 
    {
        onCleanUp();

        m_pArena->Release();
    }

    // Reset the memory heap.
    virtual void reset() { }

    // Free memory resource from the allocator.
    virtual void free(MemoryResource* pResource) { }

    // Allocate memory on the heap.
    virtual void allocate
        (
            ID3D12Device* pDevice,
            // Pointer resource, as output.
            MemoryResource** ppResource, 
            // description struct request, as input.
            const D3D12_RESOURCE_DESC* pDesc,
            // Initial state for the resource to allocate as.
            D3D12_RESOURCE_STATES initState
        ) 
    { 
    }

    // further on initializing.
    virtual bool onInitialize(size_t regionSzBytes) { return true; }

    // Further on cleaning up.
    virtual void onCleanUp() { }

    D3D12_HEAP_DESC getDesc() { return m_pArena->GetDesc(); }

protected:
    ID3D12Heap* m_pArena;
    size_t m_maxSzBytes;
    
};


// Buddy Allocation derives as far back as 1960, with region based memory management, where we split our
// memory into consistent page sizes (normally 64kb) and determine the best fitting size to store into the 
// data structure. A simple memory management data structure to implement, it proves to be the default standard
// for memory management in any high performing applications
class BuddyAllocator : public MemoryPool
{
public:

    bool onInitialize(size_t regionSzBytes) override {
        // Check for power of 2 memory size!
        ASSERT(((regionSzBytes & (regionSzBytes - 1)) == 0));

        // Doing a standard 64kb page size.
        m_minBlockSz = KB_1 * 64;

        // We are just trying to find the highest order of power of 2.
        for (U64 i = 0; i < 64; ++i) {
            U64 p = 1ull << i;
            if (p >= regionSzBytes) {
                m_maxOrder = i;
                break;
            }
        }
    }

    void allocate
        (
            // Device d3d12.
            ID3D12Device* pDevice,
            // Resource pointer as output.
            MemoryResource** ppResource,
            // Description of the resource allocation request as input. 
            const D3D12_RESOURCE_DESC* pDesc,
            // Initial state for the resource to allocate as.
            D3D12_RESOURCE_STATES initState
        ) override 
    { 
        
    }

    void free(MemoryResource* pResource) override { }

    void reset() override {
    }

private:
    struct BuddyBlock {
        size_t _memSz;
        size_t _offset;
    };

    BuddyBlock* m_pRoot;

    size_t m_maxOrder;
    size_t m_baseOffset;
    size_t m_minBlockSz;

    // Free blocks stored in the order they are found.
    std::vector<std::vector<BuddyBlock>> m_freeBlocks;

    // Allocated blocks.
    std::vector<BuddyBlock> m_allocatedBlocks;
};


/*
    Linear allocation is similiar to stack data structure. Push in FiFO standard, which is to push a memory 
    request at the top and allocate available space in the memory chunk. Keep allocating this way until there is 
    no more room in the chunk. Two ways to free memory are to clear the whole thing, or pop the top most allocated 
    portion at a time. This works for scoping gpu work such as frame resources, or quick resources, since this is the
    fastest way to allocate memory.
*/
class LinearAllocator : public MemoryPool
{
public:
    void allocate
        (
            ID3D12Device* pDevice, 
            MemoryResource** ppResource, 
            const D3D12_RESOURCE_DESC* pDesc,
            // Initial state for the resource to allocate as.
            D3D12_RESOURCE_STATES initState
        ) override 
    { 
    }

    void free(MemoryResource* pResource) override { }
    void reset() override { }
private:
};


class MemoryAllocatorD3D12 
{
public:
    void initialize
        (
            ID3D12Device* pDevice
        ) 
    {
        m_garbageIndex = 0;
    }


    ID3D12Resource* allocate
        (
            // Device d3d12 native.
            ID3D12Device* pDevice, 
            // Resource usage.
            ResourceUsage usage,
            // Initial state of the resource.
            D3D12_RESOURCE_STATES initState,
            // Resource desc. 
            const D3D12_RESOURCE_DESC& desc
        ) 
    {
        ID3D12Resource* pResource = nullptr;
        D3D12_RESOURCE_ALLOCATION_INFO info = pDevice->GetResourceAllocationInfo(0, 1, &desc);
        
        for (U32 i = 0; i < m_memPools.size(); ++i) {
            D3D12_HEAP_DESC heapDesc = m_memPools[i]->getDesc();
            
        }
        return pResource; 
    }

    void free
        (
            ID3D12Resource* pResource
        ) 
    { 
    }


private:
    std::vector<MemoryPool*> m_memPools;
    U32 m_garbageIndex;
};
} // gfx