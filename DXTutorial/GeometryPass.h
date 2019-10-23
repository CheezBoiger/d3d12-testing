//
#include "BackendRenderer.h"
#include "FrontEndRenderer.h"

namespace jcl {
    


class GeometryPass
{
public:
    void initialize();
    void cleanUp();
    RenderGroup generateCommands(GeometryMesh* pMeshes, U32 meshCount) { return RenderGroup(); }

    void setGBuffer(GBuffer* pass) { _pGBuffer = pass; }

private:
    GBuffer* _pGBuffer;
};
}