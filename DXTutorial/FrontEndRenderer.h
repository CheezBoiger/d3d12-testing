#pragma once

#include "BackendRenderer.h"
#include <vector>
#include "GlobalDef.h"
#include "VelocityRenderer.h"
#include "LightRenderer.h"
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

    THIS DOESN'T GARUANTEE THAT GRAPHICS PROGRAMMERS ARE FREE FROM THE HARDWARE THOUGH!!! Technology keeps growing,
    so must the underlying backend module!
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

    Globals* getGlobals() const { return m_pGlobals; }
    void setGlobals(Globals* pGlobals) { m_pGlobals = pGlobals; }

    void init(HWND winHandle, RendererRHI rhi);

    void cleanUp();

    void render();
    
    void update(R32 dt, Globals& globals);

    void pushMesh(GeometryMesh* pMesh, GeometrySubMesh** submeshes) { 
        m_opaqueBatches.push_back(pMesh); 
        for (U32 i = 0; i < pMesh->_submeshCount; ++i) {
            m_opaqueSubmeshes.push_back(submeshes[i]);
        }
    }

    gfx::DepthStencilView* getSceneDepthView() { return m_pSceneDepthView; }

    gfx::ShaderResourceView* getSceneResourceView() { return m_pSceneDepthResourceView; }

    gfx::DescriptorTable* getResourceDescriptorTable() { return m_pResourceDescriptorTable; }
    // Transform Buffer is the GPU buffer that is used to render the mesh. It is 
    // written to by the given mesh descriptor.
    RenderUUID createTransformBuffer();
    RenderUUID createMaterialBuffer();
    IndexBuffer createIndexBufferView(void* raw, U64 szBytes);
    RenderUUID createTexture2D(U64 width, U64 height, void* pData, DXGI_FORMAT format);

    RenderUUID createBuffer(gfx::ResourceUsage usage, gfx::ResourceBindFlags flags, U64 sz, U64 strideBytes, const TCHAR* debug);
    VertexBuffer createVertexBuffer(void* meshRaw, U64 vertexSzBytes, U64 meshSzBytes);
    RenderUUID createTexture(   gfx::ResourceDimension dimension, 
                                gfx::ResourceUsage usage, 
                                gfx::ResourceBindFlags binds, 
                                DXGI_FORMAT format, 
                                U64 width, 
                                U64 height, 
                                U64 depth, 
                                U64 strideBytes, 
                                const TCHAR* debugName);

    gfx::Resource* getGlobalsBuffer() { return pGlobalsBuffer; }

    RenderUUID createSampler();

private:
    PerMeshDescriptor mm;
    PerMeshDescriptor mm2;
    Globals* m_pGlobals;
    void beginFrame();
    void createGraphicsPipelines();
    void createFinalPipeline();
    void createFinalRootSignature();
    void createComputePipelines();
    void endFrame();

    gfx::BackendRenderer* m_pBackend;
    gfx::CommandList* m_pList;
    GBuffer m_gbuffer;
    gfx::Resource* pGlobalsBuffer;
    gfx::Resource* pMeshBuffer;
    gfx::Resource* pOtherMeshBuffer;
    gfx::Resource* m_pSceneDepth;

    gfx::DescriptorTable* m_pResourceDescriptorTable;
    gfx::RootSignature* m_pRootSignature;
    gfx::RootSignature* m_pBitonicSortSig;

    gfx::RenderTargetView* m_pAlbedoRenderTargetView;
    gfx::ShaderResourceView* m_pAlebdoShaderResourceView;
    gfx::DepthStencilView* m_pSceneDepthView;
    gfx::ShaderResourceView* m_pSceneDepthResourceView;

    gfx::GraphicsPipeline* m_pPreZPipeline;
    gfx::RenderPass* m_pPreZPass;

    GeometryPass m_geometryPass;
    Lights::LightSystem m_lightSystem;
    Shadows::LightShadow dirShadow;

    // To be run after PreZPass.
    gfx::ComputePipeline* m_clusterAssignmentPipeline;
    gfx::ComputePipeline* m_lightAssignmentPipeline;
    gfx::ComputePipeline* m_bitonicSort;

    // To be Run after Light assignement AND GBuffer pass.
    gfx::ComputePipeline* m_pLightShadingPipeline;
    gfx::ComputePipeline* m_pReflectionPipeline;
    gfx::ComputePipeline* m_pSSRPipeline;
    gfx::ComputePipeline* m_pRayTracingPipeline;
    
    // They don't have to be asyncronous.
    gfx::ComputePipeline* m_pToneMapPipeline;
    gfx::ComputePipeline* m_motionBlurPipeline;

    gfx::GraphicsPipeline* m_pFinalBackBufferPipeline;
    gfx::RootSignature* m_pFinalRootSig;
    gfx::DescriptorTable* m_pFinalDescriptorTable;

    // Define your transparent meshes, sort them from your opaques.
    std::vector<GeometryMesh*> m_transparentBatches;
    std::vector<GeometrySubMesh*> m_transparentSubmeshes;

    // Define your opaque meshes, sort them from your transparents. 
    std::vector<GeometryMesh*> m_opaqueBatches;
    std::vector<GeometrySubMesh*> m_opaqueSubmeshes; 

    // RenderGroups define the pass set for this particular set of calls.
    // Should only be setting resize on amortized time.
    std::vector<RenderGroup*> m_renderGroups;
};
} //