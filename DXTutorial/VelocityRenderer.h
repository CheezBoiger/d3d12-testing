//
#pragma once
#include "GlobalDef.h"
#include "BackendRenderer.h"

namespace jcl {

void initializeVelocityRenderer(gfx::BackendRenderer* pRenderer, gfx::DepthStencilView* pDepth);
void cleanUpVelocityRenderer(gfx::BackendRenderer* pRenderer);

void submitVelocityCommands(gfx::BackendRenderer* pRenderer, 
                            gfx::Resource* pGlobal, 
                            gfx::CommandList* pList, 
                            GeometryMesh** pMeshes, 
                            U32 meshCount,
                            GeometrySubMesh** pSubMeshes,
                            U32 submeshCount);
} // jcl