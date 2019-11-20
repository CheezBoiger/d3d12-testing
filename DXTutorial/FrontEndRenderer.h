#pragma once

#include "BackendRenderer.h"
#include <vector>
#include "GlobalDef.h"
#include "GeometryPass.h"

#include <unordered_map>

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

    gfx::BackendRenderer* getBackendRenderer() { return m_pBackend; }

    void init(HWND winHandle, RendererRHI rhi);

    void cleanUp();

    void render();
    
    void update(R32 dt, Globals& globals);

    void pushMesh(GeometryMesh* pMesh) { }

    gfx::Resource* getResource(RenderUUID uuid) { return m_pGraphicsResources[uuid]; }

    gfx::VertexBufferView* getVertexBufferView(RenderUUID uuid) { return m_pVertexBufferViews[uuid]; }

    gfx::IndexBufferView* getIndexBufferView(RenderUUID uuid) { return m_pIndexBufferViews[uuid]; }

    gfx::DepthStencilView* getSceneDepthView() { return m_pSceneDepthView; }

    gfx::ShaderResourceView* getSceneResourceView() { return m_pSceneDepthResourceView; }

    RenderUUID createBuffer(gfx::ResourceUsage usage, gfx::ResourceBindFlags flags, U64 sz, U64 strideBytes, const TCHAR* debug) { return 0; }

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

    // To be run after PreZPass.
    gfx::ComputePipeline* m_clusterAssignmentPipeline;
    gfx::ComputePipeline* m_lightAssignmentPipeline;

    // To be Run after Light assignement AND GBuffer pass.
    gfx::ComputePipeline* m_pLightShadingPipeline;

    // They don't have to be asyncronous.
    gfx::ComputePipeline* m_pToneMapPipeline;
    gfx::ComputePipeline* m_motionBlurPipeline;

    // Define your transparent meshes, sort them from your opaques.
    std::vector<GeometryMesh> m_transparentMeshes;

    // Define your opaque meshes, sort them from your transparents. 
    std::vector<GeometryMesh> m_opaqueMeshes;

    // RenderGroups define the pass set for this particular set of calls.
    // Should only be setting resize on amortized time.
    std::vector<RenderGroup*> m_renderGroups;

    std::unordered_map<RenderUUID, gfx::Resource*> m_pGraphicsResources;
    std::unordered_map<RenderUUID, gfx::VertexBufferView*> m_pVertexBufferViews;
    std::unordered_map<RenderUUID, gfx::IndexBufferView*> m_pIndexBufferViews;
};
} //