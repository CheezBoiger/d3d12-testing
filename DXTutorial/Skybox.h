#pragma once

#include "WinConfigs.h"
#include "BackendRenderer.h"
#include "FrontEndRenderer.h"

namespace jcl {
namespace Skybox {


void initialize(gfx::BackendRenderer* pRenderer);
void cleanUp(gfx::BackendRenderer* pRenderer);

void recordCommandList(gfx::CommandList* pList);
} // Skybox
} // jcl