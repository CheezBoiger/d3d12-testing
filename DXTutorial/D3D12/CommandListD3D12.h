//
#pragma once

#include "../Renderer.h"
#include "CommonsD3D12.h"


namespace gfx {

class GraphicsCommandListD3D12 : CommandList
{
public:
    GraphicsCommandListD3D12(ID3D12Device* pDevice, 
                             D3D12_COMMAND_LIST_TYPE type, 
                             ID3D12CommandAllocator* pAlloc)
        : m_pAllocatorRef(pAlloc)
        , m_type(type)
        , m_pDevice(pDevice)
        , CommandList()
    {
    }

    ~GraphicsCommandListD3D12() { }

    void init() override {
        m_pDevice->CreateCommandList(   0, 
                                        m_type, 
                                        m_pAllocatorRef, 
                                        nullptr, 
                                        __uuidof(ID3D12GraphicsCommandList), 
                                        (void**)&m_pCmdList);
    }

    void destroy() override {
        m_pCmdList->Release();
    }

    void reset() override {
        m_pCmdList->Reset(m_pAllocatorRef, nullptr);
    }

    void drawIndexedInstanced(U32 indexCountPerInstance, 
                              U32 instanceCount, 
                              U32 startIndexLocation, 
                              U32 baseVertexLocation, 
                              U32 startInstanceLocation) override {
        m_pCmdList->DrawIndexedInstanced(indexCountPerInstance, 
                                         instanceCount, 
                                         startIndexLocation, 
                                         baseVertexLocation, 
                                         startInstanceLocation);
    }

    void drawInstanced(U32 vertexCountPerInstance, 
                       U32 instanceCount, 
                       U32 startVertexLocation, 
                       U32 startInstanceLocation) override {
        m_pCmdList->DrawInstanced(vertexCountPerInstance, 
                                  instanceCount, 
                                  startVertexLocation, 
                                  startInstanceLocation);
    }

    void setPipelineStateObject() override;

    void setRenderPass(RenderPass* pass) override { }

    void dispatch(U32 x, U32 y, U32 z) override {
        m_pCmdList->Dispatch(x, y, z);
    }

    void setVertexBuffers(Buffer** buffers, U32 vertexBufferCount) override {
        //m_pCmdList->IASetVertexBuffers
        
    }

    void setIndexBuffer(Buffer** buffer) override;

    void close() override {
        m_pCmdList->Close();
    }

    void setViewports(Viewport* pViewports, U32 viewportCount) override {  
        static D3D12_VIEWPORT kNativeViewports[16];
        for (U32 i = 0; i < viewportCount; ++i) {
            Viewport& vp = pViewports[i];
            kNativeViewports[i] = { vp.x, 
                                    vp.y,
                                    vp.w,
                                    vp.h,
                                    vp.mind,
                                    vp.maxd };
        }

        m_pCmdList->RSSetViewports(viewportCount, kNativeViewports);
    }

    void setScissors(Scissor* pScissors, U32 scissorCount) override {
        static D3D12_RECT kNativeScissors[32];
        for (U32 i = 0; i < scissorCount; ++i) {
            Scissor& sr = pScissors[i];
            kNativeScissors[i] = {  sr.left,
                                    sr.top,
                                    sr.right,
                                    sr.bottom };
        }

        m_pCmdList->RSSetScissorRects(scissorCount, kNativeScissors);
    }

    void setDescriptorTables(DescriptorTable** pTables, U32 tableCount) override;

private:
    ID3D12GraphicsCommandList* m_pCmdList;
    ID3D12CommandAllocator* m_pAllocatorRef;
    ID3D12Device* m_pDevice;
    D3D12_COMMAND_LIST_TYPE m_type;
};
} // gfx