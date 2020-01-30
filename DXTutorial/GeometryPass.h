//
#pragma once 

#include "BackendRenderer.h"
#include "GlobalDef.h"

namespace jcl {
    
class GeometryMesh;
class RenderGroup;
class FrontEndRenderer;

class GeometryPass
{
public:
    void initialize(gfx::BackendRenderer* pBackend);
    void cleanUp(gfx::BackendRenderer* pBackend);
    
    void generateCommands(FrontEndRenderer* pRenderer, 
                            gfx::CommandList* pList, 
                            GeometryMesh** pMeshes, 
                            U32 meshCount,
                            GeometrySubMesh** pSubMeshes,
                            U32 submeshCount);

    void setGBuffer(GBuffer* pass) { _pGBuffer = pass; }

private:
    RenderGroup m_renderGroup;
    GBuffer* _pGBuffer;
    gfx::Sampler* m_pSampler;
    gfx::DescriptorTable* m_pSamplerTable;
    gfx::GraphicsPipeline* m_pPSO;  
    gfx::RootSignature* m_pRootSignature;
};
}