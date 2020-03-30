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

    void setMetalFactor(R32 f) { m_metalFactor = f; }
    void setRoughFactor(R32 f) { m_roughFactor = f; }

    void setAlbedoId(RenderUUID albedo) { m_albedoId = albedo; }
    void setRoughMetalId(RenderUUID roughMetal) { m_roughMetalId = roughMetal; }
    void setNormalId(RenderUUID normal) { m_normalId = normal; }
    void setEmissiveId(RenderUUID emissive) { m_emissionId = emissive; }

    RenderUUID getAlbedoSRV() const { return m_albedoSRV; }
    RenderUUID getNormalSRV() const { return m_normalSRV; }
    RenderUUID getRoughMetalSRV() const { return m_roughMetalSRV; }
    RenderUUID getEmissiveSRV() const { return m_emissionSRV; }

    RenderUUID getAlbedoId() const { return m_albedoId; }
    RenderUUID getRoughMetalId() const { return m_roughMetalId; }
    RenderUUID getNormalId() const { return m_normalId; }
    RenderUUID getEmissiveId() const { return m_emissionId; }

    R32 getMetalFactor() const { return m_metalFactor; }
    R32 getRoughFactor() const { return m_roughFactor; }

private:
    RenderUUID m_albedoId;
    RenderUUID m_roughMetalId;
    RenderUUID m_normalId;
    RenderUUID m_emissionId;

    RenderUUID m_albedoSRV;
    RenderUUID m_roughMetalSRV;
    RenderUUID m_normalSRV;
    RenderUUID m_emissionSRV;

    R32 m_metalFactor;
    R32 m_roughFactor;
    Vector3 m_albedo;
    Vector3 m_roughMetallicValue;
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
    U32 getTotalSubmeshes() const { return static_cast<U32>(m_submeshes.size()); }
    U32 getTotalVertices() const { return m_totalVertices; }
    U32 getTotalIndices() const { return m_totalIndices; }

    Bounds3D getBounds() const { return m_bounds; }

private:

    void processGLTF(const std::string& path, FrontEndRenderer* pRenderer);
    void processOBJ(const std::string& path, FrontEndRenderer* pRenderer);

    VertexBuffer m_vertexBuffer;
    IndexBuffer m_indexBuffer;
    
    U32 m_totalVertices;
    U32 m_totalIndices;
    Bounds3D m_bounds;

    std::vector<SubMesh> m_submeshes;
    std::vector<Material> m_materials;
    std::vector<RenderUUID> m_textures;
    std::vector<RenderUUID> m_samplers;
};
} // namespace