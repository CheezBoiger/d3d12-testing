#pragma once

#include "BackendRenderer.h"
#include <vector>
#include "GlobalDef.h"
#include "GeometryPass.h"

namespace jcl {


/*/
    Front End Renderer is the front end rendering engine, whose sole responsibility
    is to run the application as programmed for the game. Graphics Programmers mainly
    work here with freedom of the hardware graphics API, solely to implement lighting
    techniques, animation, particles, physics, gobos, shadows, post-processing, etc; in short,
    figure out what they should be rendering, since they need to be able to represent concept to the hardware. 
    Different renderers can be implemented for different games, but the underlying workhorse will always be the 
    BackendRenderer handling the hardware implementation.
*/
class FrontEndRenderer
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
    
    void update(R32 dt, Globals& globals);

    void pushMesh(GeometryMesh* pMesh) { }

private:
    PerMeshDescriptor mm;
    PerMeshDescriptor mm2;
    void beginFrame();
    void createGraphicsPipelines();
    void endFrame();

    gfx::BackendRenderer* m_pBackend;
    gfx::CommandList* m_pList;
    GBuffer m_gbuffer;
    gfx::Resource* pGlobalsBuffer;
    gfx::Resource* pMeshBuffer;
    gfx::Resource* pOtherMeshBuffer;
    gfx::Resource* m_pSceneDepth;

    gfx::DescriptorTable* m_pConstBufferTable;
    gfx::RootSignature* m_pRootSignature;

    gfx::RenderTargetView* m_pAlbedoRenderTargetView;
    gfx::ShaderResourceView* m_pAlebdoShaderResourceView;
    gfx::DepthStencilView* m_pSceneDepthView;
    gfx::ShaderResourceView* m_pSceneDepthResourceView;

    gfx::Resource* m_pTriangleVertexBuffer;
    gfx::VertexBufferView* m_pTriangleVertexBufferView;
    gfx::GraphicsPipeline* m_pPreZPipeline;
    gfx::RenderPass* m_pPreZPass;

    GeometryPass m_geometryPass;

    // Define your transparent meshes, sort them from your opaques.
    std::vector<GeometryMesh> m_transparentMeshes;

    // Define your opaque meshes, sort them from your transparents. 
    std::vector<GeometryMesh> m_opaqueMeshes;

    // RenderGroups define the pass set for this particular set of calls.
    std::vector<RenderGroup*> m_renderGroups;

    void retrieveShader(const std::string& filepath, void** bytecode, size_t& length);
};
} //