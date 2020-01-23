#pragma once

#include "BackendRenderer.h"
#include "Math/Vector4.h"

namespace jcl {


void initDebugGUI( gfx::BackendRenderer* pRenderer, HWND handle );
void cleanUpDebugGUI( gfx::BackendRenderer* pRenderer );

void populateCommandListGUI( gfx::BackendRenderer* pRenderer, gfx::CommandList* pCmdList);


void updateMouseCursor( R32 x, R32 y );
}