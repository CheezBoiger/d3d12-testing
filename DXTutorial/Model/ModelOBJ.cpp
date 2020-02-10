//
#include "Model.h"
#include "../GlobalDef.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "../Math/Bounds3D.h"

#include <vector>
#include <cmath>

namespace jcl {


void Model::processOBJ(const std::string& path, FrontEndRenderer* pRenderer)
{
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    tinyobj::attrib_t attrib;
    std::string warn;
    std::string err;

    bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!warn.empty()) {
        // do some warns.
    }

    if (!err.empty()) {
        // do err logging.
        return;
    }

    if (!result) {
        return;
    }
    // We calculate the bounds of the mesh to determine if it is not centered at the origin.
    Bounds3D bounds(Vector3(FLT_MAX, FLT_MAX, FLT_MAX), Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX));
    std::vector<Vertex> vertices;
    std::vector<U32> indices;

    // Each shape is a submesh.
    size_t indexOffset = 0;
    size_t vertexOffset = 0;
    for (size_t s = 0; s < shapes.size(); ++s) {
        size_t index_offset = 0;
        SubMesh submesh;
        size_t indexCount = 0;
        size_t vertexCount = 0;

        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
            I32 fv = shapes[s].mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                ++vertexCount;
                ++indexCount;
                Vertex vert;
                vert._position._x = attrib.vertices[3 * idx.vertex_index + 0];
                vert._position._y = -attrib.vertices[3 * idx.vertex_index + 1];
                vert._position._z = attrib.vertices[3 * idx.vertex_index + 2];
                vert._position._w = 1.0f;
                vert._normal._x = -attrib.normals[3 * idx.normal_index + 0];
                vert._normal._y = attrib.normals[3 * idx.normal_index + 1];
                vert._normal._z = -attrib.normals[3 * idx.normal_index + 2];
                vert._normal._w = 1.0f;
                vert._texcoords._x = attrib.texcoords[2 * idx.texcoord_index + 0];
                vert._texcoords._y = attrib.texcoords[2 * idx.texcoord_index + 1];
                vert._texcoords._z = 0.0f;
                vert._texcoords._w = 0.0f;

                vertices.push_back(vert);
                indices.push_back(indices.size());

                bounds._max = Vector3(fmaxf(bounds._max._x, vert._position._x),
                                      fmaxf(bounds._max._y, vert._position._y),
                                      fmaxf(bounds._max._z, vert._position._z));
                bounds._min = Vector3(fminf(bounds._min._x, vert._position._x),
                                      fminf(bounds._min._y, vert._position._y),
                                      fminf(bounds._min._z, vert._position._z));
            }

            index_offset += fv;
            // Per face material.
            I32 matId = shapes[s].mesh.material_ids[f];
            //auto& mtl = materials[matId];
        }

        submesh.m_indCount = indexCount;
        submesh.m_indOffset = indexOffset;
        submesh.m_vertCount = vertexCount;
        submesh.m_vertOffset = vertexOffset;
        vertexOffset += vertexCount;
        indexOffset += indexCount;
        m_submeshes.push_back(submesh);
    }

    if (bounds.getCenter() != Vector3(0.f, 0.f, 0.f)) {
        Vector3 diff = -bounds.getCenter();
        for (size_t v = 0; v < vertices.size(); ++v) {
            Vertex& vertex = vertices[v];
            vertex._position._x = vertex._position._x + diff._x;
            vertex._position._y = vertex._position._y + diff._y;
            vertex._position._z = vertex._position._z + diff._z;
        }
    }

    m_vertexBuffer = pRenderer->createVertexBuffer(vertices.data(), sizeof(Vertex), sizeof(Vertex) * vertices.size());
    m_indexBuffer = pRenderer->createIndexBufferView(indices.data(), indices.size() * sizeof(U32));
    m_totalIndices = indices.size();
    m_totalVertices = vertices.size();
}
} // jcl