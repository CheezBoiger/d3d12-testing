//
#include "LightRenderer.h"
#include "BackendRenderer.h"
#include "ShadowRenderer.h"

namespace jcl {
namespace Lights {


gfx::Resource* lightOutputResource = nullptr;

gfx::RenderTargetView* lightOutputRTV = nullptr;
gfx::ShaderResourceView* lightOutputSRV = nullptr;
gfx::UnorderedAccessView* lightOutputUAV = nullptr;

gfx::ComputePipeline* lightDeferredPipeline = nullptr;
gfx::RootSignature* lightDeferredRootSignature = nullptr;

LightSystem* pLightSystemUse = nullptr;
gfx::DescriptorTable* lightDeferredDescriptorTable = nullptr;

void LightSystem::initialize(gfx::BackendRenderer* pRenderer, 
                            U32 directionLightCount, 
                            U32 pointLightCount, 
                            U32 spotLightCount)
{
    struct {
        float _0[4];
        float _1[4];
        float _2[4];
        int _3[4];
    } DirLight;
    struct {
        float _0[4];
        float _1[4];
        float _2[4];
        float _3[4];
    } RSpotLight;
    struct {
        float _0[4];
        float _1[4];
        float _2[4];
    } RPointLight;
    pRenderer->createBuffer(&m_pDirectionLightResource, 
                            gfx::RESOURCE_USAGE_CPU_TO_GPU,
                            gfx::RESOURCE_BIND_SHADER_RESOURCE,
                            sizeof(DirLight) * directionLightCount, 0, TEXT("DirectionLightBuffer"));
    pRenderer->createBuffer(&m_pSpotLightResource, 
                            gfx::RESOURCE_USAGE_CPU_TO_GPU,
                            gfx::RESOURCE_BIND_SHADER_RESOURCE,
                            sizeof(DirLight) * spotLightCount, 0, TEXT("SpotLightBuffer"));
    pRenderer->createBuffer(&m_pPointLightResource, 
                            gfx::RESOURCE_USAGE_CPU_TO_GPU,
                            gfx::RESOURCE_BIND_SHADER_RESOURCE,
                            sizeof(DirLight) * pointLightCount, 0, TEXT("PointLightBuffer"));
    pRenderer->createBuffer(&m_lightTransformsResource,
                            gfx::RESOURCE_USAGE_CPU_TO_GPU,
                            gfx::RESOURCE_BIND_SHADER_RESOURCE,
                            256 * (directionLightCount + pointLightCount + spotLightCount), 
                            0, TEXT("LightTransforms"));
    gfx::ShaderResourceViewDesc srvDesc = { };
    srvDesc._dimension = gfx::SRV_DIMENSION_BUFFER;
    srvDesc._format = DXGI_FORMAT_UNKNOWN;
    srvDesc._buffer._firstElement = 0;
    srvDesc._buffer._numElements = directionLightCount;
    srvDesc._buffer._structureByteStride = sizeof(DirLight);
    pRenderer->createShaderResourceView(&m_pDirectionLightsSRV, m_pDirectionLightResource, srvDesc);
    srvDesc._buffer._numElements = pointLightCount;
    srvDesc._buffer._structureByteStride = sizeof(RPointLight);
    pRenderer->createShaderResourceView(&m_pPointLighstSRV, m_pPointLightResource, srvDesc);
    srvDesc._buffer._numElements = spotLightCount;
    srvDesc._buffer._structureByteStride = sizeof(RSpotLight);
    pRenderer->createShaderResourceView(&m_pSpotLightsSRV, m_pSpotLightResource, srvDesc);
    srvDesc._buffer._numElements = directionLightCount + pointLightCount + spotLightCount;
    srvDesc._buffer._structureByteStride = 256;
    pRenderer->createShaderResourceView(&m_pLightTransformSRV, m_lightTransformsResource, srvDesc);

    m_directionLights.resize(directionLightCount);
    m_pointLights.resize(pointLightCount);
    m_spotLights.resize(spotLightCount);
    m_lightTransformations.resize(directionLightCount + pointLightCount + spotLightCount);

    U64 idx = 0;
    for (U32 i = 0; i < directionLightCount; ++i)
        m_directionLights[i]._transform = idx++;
    for (U32 i = 0; i < pointLightCount; ++i)
        m_pointLights[i]._transform = idx++;
    for (U32 i = 0; i < spotLightCount; ++i)
        m_spotLights[i]._transform = idx++;
}


void createRootDescriptor(gfx::BackendRenderer* pRenderer)
{
    gfx::PipelineLayout layouts[1];
    layouts[0] = { };
    layouts[0]._type = gfx::PIPELINE_LAYOUT_TYPE_DESCRIPTOR_TABLE; 
    layouts[0]._numShaderResourceViews = 13;
    layouts[0]._numUnorderedAcessViews = 1;
    layouts[0]._numConstantBuffers = 1;
    pRenderer->createRootSignature(&lightDeferredRootSignature);
    lightDeferredRootSignature->initialize(gfx::SHADER_VISIBILITY_ALL,
                                           layouts, 1);
}


void createComputePipeline(gfx::BackendRenderer* pRenderer)
{
    gfx::ComputePipelineInfo info = { };
    info._computeShader._pByteCode = new U8[5 * 1024 * 1024];
    info._pRootSignature = lightDeferredRootSignature;
    retrieveShader("ComputeLighting.cs.cso", &info._computeShader._pByteCode, info._computeShader._szBytes);
    pRenderer->createComputePipelineState(&lightDeferredPipeline, &info);
    delete[] info._computeShader._pByteCode;
}

gfx::ShaderResourceView* getLightOutputSRV()
{
    return lightOutputSRV;
}


void createDescriptorTables(gfx::BackendRenderer* pRenderer)
{
    pRenderer->createDescriptorTable(&lightDeferredDescriptorTable);
    lightDeferredDescriptorTable->initialize(gfx::DescriptorTable::DESCRIPTOR_TABLE_SRV_UAV_CBV, 15);
}


void initializeLights(gfx::BackendRenderer* pRenderer)
{
    pRenderer->createTexture(&lightOutputResource,
                             gfx::RESOURCE_DIMENSION_2D,
                             gfx::RESOURCE_USAGE_DEFAULT,
                             gfx::RESOURCE_BIND_RENDER_TARGET | gfx::RESOURCE_BIND_SHADER_RESOURCE,
                             DXGI_FORMAT_R8G8B8A8_UNORM, 1920, 1080, 1, 0, TEXT("LightOutput"));
    gfx::RenderTargetViewDesc rtvDesc = { };
    rtvDesc._dimension = gfx::RTV_DIMENSION_TEXTURE_2D;
    rtvDesc._texture2D._mipSlice = 0;
    rtvDesc._texture2D._planeSlice = 0; 
    rtvDesc._format = DXGI_FORMAT_R8G8B8A8_UNORM;
    pRenderer->createRenderTargetView(&lightOutputRTV,
                                      lightOutputResource,
                                      rtvDesc);
    gfx::ShaderResourceViewDesc srvDesc = { };
    srvDesc._dimension = gfx::SRV_DIMENSION_TEXTURE_2D;
    srvDesc._format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc._texture2D._mipLevels = 1;
    srvDesc._texture2D._mostDetailedMip = 0;
    srvDesc._texture2D._planeSlice = 0;
    srvDesc._texture2D._resourceMinLODClamp = 0.0f;
    pRenderer->createShaderResourceView(&lightOutputSRV, 
                                        lightOutputResource, 
                                        srvDesc);
    gfx::UnorderedAccessViewDesc uavDesc = { };
    uavDesc._dimension = gfx::UAV_DIMENSION_TEXTURE_2D;
    uavDesc._format = DXGI_FORMAT_R8G8B8A8_UNORM;
    uavDesc._texture2D._mipSlice = 0;
    uavDesc._texture2D._planeSlice = 0;
    pRenderer->createUnorderedAccessView(&lightOutputUAV, 
                                         lightOutputResource, 
                                         uavDesc);
    createRootDescriptor(pRenderer);
    createComputePipeline(pRenderer);
    createDescriptorTables(pRenderer);
}


void updateLightRenderer
    (
        // Global constant buffer.
        gfx::Resource* pGlobalConstBuffer,
        // Deferred GBuffer descriptor table.
        GBuffer* gpass,
        // Light system to use.
        LightSystem* pLightSystem
    )
{
    gfx::ShaderResourceView* pointLightSRV = getPointLightsSRV(pLightSystem);
    gfx::ShaderResourceView* spotLightSRV = getSpotLightsSRV(pLightSystem);
    gfx::ShaderResourceView* directionLightSRV = getDirectionLightsSRV(pLightSystem);
    gfx::ShaderResourceView* srvs[] = { 
        gpass->pAlbedoSRV,
        gpass->pNormalSRV,
        gpass->pMaterialSRV,
        gpass->pEmissiveSRV,
        // Missing Light Transformations!
        pointLightSRV, 
        spotLightSRV, 
        directionLightSRV
        // Missing Shadows!
    };
    lightDeferredDescriptorTable->setConstantBuffers(&pGlobalConstBuffer, 1);
    lightDeferredDescriptorTable->setUnorderedAccessViews(&lightOutputUAV, 1);
    lightDeferredDescriptorTable->setShaderResourceViews(srvs, 4);
    lightDeferredDescriptorTable->update(gfx::DESCRIPTOR_TABLE_FLAG_RESET);
}


void generateDeferredLightsCommands
    (
        gfx::CommandList* pList,
        gfx::Resource* global
    )
{
    if (!pList) return;
    pList->setDescriptorTables(&lightDeferredDescriptorTable, 1);
    pList->setComputeRootSignature(lightDeferredRootSignature);
    pList->setComputePipeline(lightDeferredPipeline);
    pList->setComputeRootDescriptorTable(0, lightDeferredDescriptorTable);
    pList->dispatch(1920 / 16 + 1, 1080 / 16 + 1, 1);
}


gfx::ShaderResourceView* getPointLightsSRV(LightSystem* pLightSystem)
{
    return pLightSystem->m_pPointLighstSRV;
}


gfx::ShaderResourceView* getSpotLightsSRV(LightSystem* pLightSystem)
{
    return pLightSystem->m_pSpotLightsSRV;
}


gfx::ShaderResourceView* getDirectionLightsSRV(LightSystem* pLightSystem)
{
    return pLightSystem->m_pDirectionLightsSRV;
}


void LightSystem::mapLightSystem(void** dirPtr, void** pointPtr, void** spotPtr, void** transformPtr)
{
    struct {
        float _0[4];
        float _1[4];
        float _2[4];
        int _3[4];
    } DirLight;
    struct {
        float _0[4];
        float _1[4];
        float _2[4];
        float _3[4];
    } RSpotLight;
    struct {
        float _0[4];
        float _1[4];
        float _2[4];
    } RPointLight;

    
    *dirPtr = m_pDirectionLightResource->map(nullptr);
    *pointPtr = m_pPointLightResource->map(nullptr);
    *spotPtr = m_pSpotLightResource->map(nullptr);
    *transformPtr = m_lightTransformsResource->map(nullptr);
}


void LightSystem::unmapLightSystem()
{
    m_pDirectionLightResource->unmap(nullptr);
    m_pSpotLightResource->unmap(nullptr);
    m_pPointLightResource->unmap(nullptr);
}


void LightSystem::update()
{ 
    struct {
        float _0[4];
        float _1[4];
        float _2[4];
        int _3[4];
    } DirLight;
    struct {
        float _0[4];
        float _1[4];
        float _2[4];
        float _3[4];
    } RSpotLight;
    struct {
        float _0[4];
        float _1[4];
        float _2[4];
    } RPointLight;

    void* dirPtr = nullptr;
    void* pointPtr = nullptr;
    void* spotDir = nullptr;
    void* transformPtr = nullptr; 
    mapLightSystem(&dirPtr, &pointPtr, &spotDir, &transformPtr);

    for (U32 i = 0; i < m_directionLights.size(); ++i) {
        *(R32*)((U8*)dirPtr + sizeof(DirLight) * i) = m_directionLights[i]._direction[0]; 
        *(R32*)((U8*)dirPtr + sizeof(DirLight) * i + 32) = m_directionLights[i]._direction[1];
        *(R32*)((U8*)dirPtr + sizeof(DirLight) * i + 64) = m_directionLights[i]._direction[2];
        *(R32*)((U8*)dirPtr + sizeof(DirLight) * i + 96) = 1.0f;
    }

    U32 idx = 0;
    for (U32 i = 0; i < m_directionLights.size() + m_spotLights.size() + m_pointLights.size(); ++i) {
        struct TransformGPU {
            Matrix44 _viewToClip;
            Matrix44 _clipToView;
        };
        TransformGPU* t = (TransformGPU*)((U8*)transformPtr + 256 * i);
        t->_clipToView = m_lightTransformations[idx]._clipToView;
        t->_viewToClip = m_lightTransformations[idx++]._viewToClip;
    }
    
    unmapLightSystem();
}


gfx::Resource* getLightTransforms(LightSystem* system)
{
    return system->m_lightTransformsResource;
}
} // Lights
} // jcl