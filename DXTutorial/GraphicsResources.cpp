//
#include "GraphicsResources.h"


namespace jcl {


class GPUCache
{
public:
    static RenderUUID cacheGPUResource(gfx::Resource* pResource);
    static gfx::Resource* getGPUResource(RenderUUID uuid);
};


RenderUUID idd = 0;


std::unordered_map<RenderUUID, gfx::Resource*> m_pGraphicsResources;
std::unordered_map<RenderUUID, gfx::VertexBufferView*> m_pVertexBufferViews;
std::unordered_map<RenderUUID, gfx::IndexBufferView*> m_pIndexBufferViews;

RenderUUID cacheResource(gfx::Resource* pResource)
{
    RenderUUID id = idd++;
    m_pGraphicsResources[id] = pResource;
    return id;
}


RenderUUID cacheVertexBufferView(gfx::VertexBufferView* pView)
{
    RenderUUID id = idd++;
    m_pVertexBufferViews[id] = pView;
    return id;
}


RenderUUID cacheIndexBufferView(gfx::IndexBufferView* pView)
{
    RenderUUID id = idd++;
    m_pIndexBufferViews[id] = pView;
    return id;
}


gfx::Resource* getResource(RenderUUID uuid) 
{ 
    return m_pGraphicsResources[uuid]; 
}


gfx::VertexBufferView* getVertexBufferView(RenderUUID uuid) 
{ 
    return m_pVertexBufferViews[uuid]; 
}


gfx::IndexBufferView* getIndexBufferView(RenderUUID uuid) 
{ 
    return m_pIndexBufferViews[uuid]; 
}
} // jcl