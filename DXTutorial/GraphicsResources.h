//
#pragma once

#include "GlobalDef.h"
#include "BackendRenderer.h"

#include <unordered_map>

namespace jcl {


RenderUUID cacheResource(gfx::Resource* pResource);
RenderUUID cacheVertexBufferView(gfx::VertexBufferView* pView);
RenderUUID cacheIndexBufferView(gfx::IndexBufferView* pView);

gfx::Resource* getResource(RenderUUID uuid);
gfx::VertexBufferView* getVertexBufferView(RenderUUID uuid);
gfx::IndexBufferView* getIndexBufferView(RenderUUID uuid);

} // jcl