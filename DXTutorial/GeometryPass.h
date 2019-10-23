//
#include "BackendRenderer.h"
#include "GlobalDef.h"

namespace jcl {
    
class GeometryMesh;
class RenderGroup;

class GeometryPass
{
public:
    void initialize(gfx::BackendRenderer* pBackend);
    void cleanUp(gfx::BackendRenderer* pBackend);
    RenderGroup* generateCommands(GeometryMesh* pMeshes, U32 meshCount) { return &m_renderGroup; }

    void setGBuffer(GBuffer* pass) { _pGBuffer = pass; }

private:
    RenderGroup m_renderGroup;
    GBuffer* _pGBuffer;

    gfx::GraphicsPipeline* m_pPSO;
};
}