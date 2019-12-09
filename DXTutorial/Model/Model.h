//
#pragma once

#include "../Math/Matrix44.h"
#include "../Math/Vector4.h"
#include "../GlobalDef.h"
#include "../FrontEndRenderer.h"

#include <string>
#include <vector>

namespace jcl {

class Material
{
public:
    Material() { }

    void enableBumpmapping(B32 enable) { m_useBumpmapping = enable; }

    void setMetalFactor(R32 f) { m_metalFactor = f; }
    void setRoughFactor(R32 f) { m_roughFactor = f; }
    void setAlbedoId(RenderUUID albedo) { m_albedoId = albedo; }
    void setRoughMetalId(RenderUUID roughMetal) { m_roughMetalId = roughMetal; }
    void setNormalId(RenderUUID normal) { m_normalId = normal; }
    void setEmissiveId(RenderUUID emissive) { m_emissionId = emissive; }

    RenderUUID getAlbedoId() const { return m_albedoId; }
    RenderUUID getRoughMetalId() const { return m_roughMetalId; }
    RenderUUID getNormalId() const { return m_normalId; }
    RenderUUID getEmissiveId() const { return m_emissionId; }

    R32 getMetalFactor() const { return m_metalFactor; }
    R32 getRoughFactor() const { return m_roughFactor; }
    B32 isUsingBumpmapping() const { return m_useBumpmapping; }

    gfx::DescriptorTable* getDescriptorTable() { }

private:
    RenderUUID m_albedoId;
    RenderUUID m_roughMetalId;
    RenderUUID m_normalId;
    RenderUUID m_emissionId;
    gfx::DescriptorTable* m_pDescriptorTable;
    B32 m_useBumpmapping;
    R32 m_metalFactor;
    R32 m_roughFactor;
};


class SubMesh
{
public:
    SubMesh() : m_vertCount(0), m_vertOffset(0) { }

    void initialize(U64 vertOffset, U64 vertCount,
                    U64 indOffset, U64 indCount, Material* mat);
    U64 m_vertOffset;
    U64 m_vertCount;
    U64 m_indOffset;
    U64 m_indCount;
    Material* m_materialId;
};

class Model
{
public:

    B32 initialize(const std::string& path, FrontEndRenderer* pRenderer);
    B32 cleanUp();

    RenderUUID getVertexBufferView() const { return m_vertexBuffer.vertexBufferView; }
    RenderUUID getIndexBufferView() const { return m_indexBuffer.indexBufferView; }
    SubMesh* getSubMesh(size_t i) { return &m_submeshes[i]; }

private:
    VertexBuffer m_vertexBuffer;
    IndexBuffer m_indexBuffer;
    
    std::vector<SubMesh> m_submeshes;
    std::vector<Material> m_materials;
    std::vector<RenderUUID> m_textures;
    std::vector<RenderUUID> m_samplers;
};
} // namespace