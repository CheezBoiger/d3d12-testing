#pragma once

#include "Renderer.h"
#include <vector>
#include "GlobalDef.h"

namespace jcl {


typedef U64 RenderUUID;

struct RenderCommand 
{
  RenderUUID _meshDescriptor;
  RenderUUID _materialDescriptor;
};


struct RenderGroup
{
  // The given pipeline used for this render group.
  RenderUUID _pipeline;
  // Render in deferred lighting flow.
  B32 _isDeferred;
  // All render commands to be processed.
  std::vector<RenderCommand> _renderCommands;
};

/*/
    Jackal Renderer is the front end rendering engine, whose sole responsibility
    is to run the application as programmed for the game. Graphics Programmers mainly
    work here with freedom of the hardware graphics API, solely to implement lighting
    techniques, animation, particles, physics, gobos, shadows, post-processing, etc; in short,
    figure out what they should be rendering. Different renderers can be implemented for different
    games, but the underlying workhorse will always be the BackendRenderer handling the hardware implementation.
*/
class JackalRenderer
{
public:

    enum RendererRHI {
      RENDERER_RHI_NULL,
      RENDERER_RHI_D3D_11,
      RENDERER_RHI_D3D_12
    };

    void init(HWND winHandle, RendererRHI rhi);

    void cleanUp();

    void render();
    
    void update(R32 dt) { }

    void pushRenderGroups(RenderGroup& group) { }

private:

    void beginFrame();
    void endFrame();

    gfx::BackendRenderer* m_pBackend;
    gfx::CommandList* m_pList;
    GBuffer m_gbuffer;
    gfx::Resource* pGlobalsBuffer;
    gfx::Resource* m_pSceneDepth;

    gfx::DescriptorTable* m_pConstBufferTable;
    gfx::RootSignature* m_pRootSignature;

    gfx::RenderTargetView* m_pAlbedoRenderTargetView;
    gfx::ShaderResourceView* m_pAlebdoShaderResourceView;
    gfx::DepthStencilView* m_pSceneDepthView;
    gfx::ShaderResourceView* m_pSceneDepthResourceView;
    Globals m_globals;

    gfx::Resource* m_pTriangleVertexBuffer;
    gfx::VertexBufferView* m_pTriangleVertexBufferView;
};
} //