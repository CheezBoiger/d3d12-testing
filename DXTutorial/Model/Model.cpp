//
#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"


namespace jcl {


struct Vertex
{
    struct { R32 _x, _y, _z, _w; } _position;
    struct { R32 _x, _y, _z, _w; } _normal;
    struct { R32 _x, _y, _z, _w; } _tangent;
    struct { R32 _x, _y, _z, _w; } _texcoords;
};


void loadSamplers(tinygltf::Model* pModel, gfx::BackendRenderer* pRenderer)
{
    for (U32 i = 0; i < pModel->samplers.size(); ++i) {
        tinygltf::Sampler& sampler = pModel->samplers[i];
    }
}


void loadTextures(tinygltf::Model* pModel, std::vector<gfx::Resource*>& textures, gfx::BackendRenderer* pRenderer)
{
    for (U32 i = 0; i < pModel->images.size(); ++i) {
        tinygltf::Image& image = pModel->images[i];
        gfx::Resource* pTexture = nullptr;
        
        pRenderer->createTexture(   &pTexture, 
                                    gfx::RESOURCE_DIMENSION_2D, 
                                    gfx::RESOURCE_USAGE_DEFAULT, 
                                    gfx::RESOURCE_BIND_SHADER_RESOURCE,
                                    DXGI_FORMAT_R8G8B8A8_TYPELESS,
                                    image.width, image.height, 1, 0, TEXT("ttext"));
        textures.push_back(pTexture);
    }
}


std::vector<Material> loadMaterials(tinygltf::Model* pModel, std::vector<gfx::Resource*>& textures)
{
    std::vector<Material> materials;
    for (U32 i = 0; i < pModel->materials.size(); ++i) {
        tinygltf::Material& mat = pModel->materials[i];
        Material materials;
    }
    return materials;
}


void loadNode(tinygltf::Model* pModel, tinygltf::Node& node, std::vector<Vertex>& vertices, std::vector<U32> indices, std::vector<SubMesh>& submeshes)
{
    // contains mesh.
    if (node.mesh) {
        tinygltf::Mesh& mmesh = pModel->meshes[node.mesh];
        for (U32 i = 0; i < mmesh.primitives.size(); ++i) {
            SubMesh submesh = { };
            tinygltf::Primitive& primitive = mmesh.primitives[i];
            R32* positionAttribs = nullptr;
            R32* normalAttribs = nullptr;
            R32* tangentAttribs = nullptr;
            R32* texCoordAttribs = nullptr;
    
            // POSITION should always be garuanteed.
            const tinygltf::Accessor& positionAccessor = pModel->accessors[primitive.attributes["POSITION"]];
            const tinygltf::BufferView& bufView = pModel->bufferViews[positionAccessor.bufferView];
            positionAttribs = reinterpret_cast<R32*>(pModel->buffers[bufView.buffer].data[positionAccessor.byteOffset + bufView.byteOffset]);

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const tinygltf::Accessor& normalAccessor = pModel->accessors[primitive.attributes["NORMAL"]];
                const tinygltf::BufferView& bufView = pModel->bufferViews[normalAccessor.bufferView];
                normalAttribs = reinterpret_cast<R32*>(pModel->buffers[bufView.buffer].data[normalAccessor.byteOffset + bufView.byteOffset]);
            }

            if (primitive.attributes.find("TANGENT") != primitive.attributes.end()) {
                const tinygltf::Accessor& tangentAccessor = pModel->accessors[primitive.attributes["TANGENT"]];
                const tinygltf::BufferView& bufView = pModel->bufferViews[tangentAccessor.bufferView];
                tangentAttribs = reinterpret_cast<R32*>(pModel->buffers[bufView.buffer].data[tangentAccessor.byteOffset + bufView.byteOffset]);         
            }

            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const tinygltf::Accessor& texcoordAccessor = pModel->accessors[primitive.attributes["TEXCOORD_0"]];
                const tinygltf::BufferView& bufView = pModel->bufferViews[texcoordAccessor.bufferView];    
                texCoordAttribs = reinterpret_cast<R32*>(pModel->buffers[bufView.buffer].data[texcoordAccessor.byteOffset + bufView.byteOffset]);
            }

            U64 currVertCount = vertices.size();

            for (U32 i = 0; i < positionAccessor.count; ++i) {
                Vertex vert = { };
                vert._position._x = positionAttribs[i * 3 + 0];
                vert._position._y = positionAttribs[i * 3 + 1];
                vert._position._z = positionAttribs[i * 3 + 2];
                vert._position._w = 1.0f;
        
                vert._normal._x = normalAttribs[i * 3 + 0];
                vert._normal._y = normalAttribs[i * 3 + 1];
                vert._normal._z = normalAttribs[i * 3 + 2];
                vert._normal._w = 1.0f;

                vert._tangent._x = tangentAttribs[i * 3 + 0];
                vert._tangent._y = tangentAttribs[i * 3 + 1];
                vert._tangent._z = tangentAttribs[i * 3 + 2];
                vert._tangent._w = 1.0f;

                vert._texcoords._x = texCoordAttribs[i * 2 + 0];
                vert._texcoords._y = texCoordAttribs[i * 2 + 1]; 
                vertices.push_back(vert);
            }

            const tinygltf::Accessor& indicesAccessor = pModel->accessors[primitive.indices];
            const tinygltf::BufferView& indBufView = pModel->bufferViews[indicesAccessor.bufferView];
            const tinygltf::Buffer& indBuf = pModel->buffers[indBufView.buffer];
            U64 indicesOffset = indices.size();
            U64 indicesCount = indicesAccessor.count;
            switch (indicesAccessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                    const U32* buf = (const U32*)&indBuf.data[indicesAccessor.byteOffset + indBufView.byteOffset];
                    for (U32 idx = 0; idx < indicesCount; ++idx) {
                        indices.push_back(buf[idx]);
                    }
                } break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                default: {
                } break;
            }

            submesh.initialize(currVertCount, positionAccessor.count, indicesOffset, indicesCount);
            submeshes.push_back(submesh);
        }
    }

    for (U32 child = 0; child < node.children.size(); ++child) {
        tinygltf::Node& childNode = pModel->nodes[node.children[child]];
        loadNode(pModel, childNode, vertices, indices, submeshes);
    }
}


std::vector<SubMesh> loadMeshes(tinygltf::Model* pModel, FrontEndRenderer* pRenderer, RenderUUID* pVert, RenderUUID* pInd)
{
    std::vector<Vertex> vertices;
    std::vector<U32> indices;
    std::vector<SubMesh> submeshes;

    tinygltf::Scene& scene = pModel->scenes[pModel->defaultScene];
    for (U32 i = 0; i < scene.nodes.size(); ++i) {
        tinygltf::Node& node = pModel->nodes[scene.nodes[i]];
        loadNode(pModel, node, vertices, indices, submeshes);
    }

    *pVert = pRenderer->createBuffer(   gfx::RESOURCE_USAGE_DEFAULT,
                                        gfx::RESOURCE_BIND_VERTEX_BUFFER, 
                                        vertices.size() * sizeof(Vertex), 
                                        0, 
                                        TEXT("SceneVertexBuffer"));

    *pInd = pRenderer->createBuffer(    gfx::RESOURCE_USAGE_DEFAULT,
                                        gfx::RESOURCE_BIND_INDEX_BUFFER,
                                        indices.size() * sizeof(U32),
                                        0,
                                        TEXT("SceneIndexBuffer"));

    return submeshes;
}


void SubMesh::initialize(U64 vertOffset, U64 vertCount, U64 indOffset, U64 indCount)
{
    m_vertOffset = vertOffset;
    m_vertCount = vertCount;
    m_indCount = indCount;
    m_indOffset = indOffset;
}


B32 Model::initialize(const std::string& path, FrontEndRenderer* pRenderer)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    ASSERT(ret);

    std::vector<gfx::Resource*> textureResources;
    loadTextures(&model, textureResources, pRenderer->getBackendRenderer());
    m_materials = loadMaterials(&model, textureResources);
    m_submeshes = loadMeshes(&model, pRenderer, &m_vertBufferId, &m_indBufferId);
    return false;
}


B32 Model::cleanUp()
{
    return false;
}
} // jcl