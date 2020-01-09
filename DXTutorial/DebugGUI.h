#pragma once

#include "BackendRenderer.h"


namespace jcl {


void initDebugGUI( gfx::BackendRenderer* pRenderer, HWND handle );
void populateCommandListGUI( gfx::BackendRenderer* pRenderer, gfx::CommandList* pCmdList);
}