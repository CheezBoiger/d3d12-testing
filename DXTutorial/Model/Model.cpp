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


void loadTextures(tinygltf::Model* pModel, gfx::BackendRenderer* pRenderer)
{
    for (U32 i = 0; i < pModel->textures.size(); ++i) {
        
    }
}


std::vector<Material> loadMaterials(tinygltf::Model* pModel)
{
    std::vector<Material> materials;
    pModel->materials[0];
    return materials;
}


void loadNode(tinygltf::Model* pModel, tinygltf::Node& node, std::vector<Vertex>& vertices, std::vector<U32> indices, std::vector<SubMesh>& submeshes)
{
    // contains mesh.
    if (node.mesh) {
        tinygltf::Mesh& mmesh = pModel->meshes[node.mesh];
        for (U32 i = 0; i < mmesh.primitives.size(); ++i) {
            tinygltf::Primitive& primitive = mmesh.primitives[i];
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const tinygltf::Accessor& positionAccessor = pModel->accessors[primitive.attributes["POSITION"]];
                const tinygltf::BufferView& bufView = pModel->bufferViews[positionAccessor.bufferView];
                
            }

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const tinygltf::Accessor& normalAccessor = pModel->accessors[primitive.attributes["NORMAL"]];
            }

            if (primitive.attributes.find("TANGENT") != primitive.attributes.end()) {
                const tinygltf::Accessor& tangentAccessor = pModel->accessors[primitive.attributes["TANGENT"]];
            }

            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const tinygltf::Accessor& texcoordAccessor = pModel->accessors[primitive.attributes["TEXCOORD_0"]];
            }
        }
    }

    for (U32 child = 0; child < node.children.size(); ++child) {
        tinygltf::Node& childNode = pModel->nodes[node.children[child]];
        loadNode(pModel, childNode, vertices, indices, submeshes);
    }
}


std::vector<SubMesh> loadMeshes(tinygltf::Model* pModel, gfx::BackendRenderer* pRenderer, RenderUUID* pVert, RenderUUID* pInd)
{
    std::vector<Vertex> vertices;
    std::vector<U32> indices;
    std::vector<SubMesh> submeshes;

    tinygltf::Scene& scene = pModel->scenes[pModel->defaultScene];
    for (U32 i = 0; i < scene.nodes.size(); ++i) {
        tinygltf::Node& node = pModel->nodes[scene.nodes[i]];
        loadNode(pModel, node, vertices, indices, submeshes);
    }

    gfx::Resource* pVertexBuffer = nullptr;
    pRenderer->createBuffer(&pVertexBuffer, gfx::RESOURCE_USAGE_DEFAULT, gfx::RESOURCE_BIND_VERTEX_BUFFER, vertices.size() * sizeof(Vertex), 0, TEXT("SceneBuffer"));

    return submeshes;
}


void SubMesh::initialize()
{
}


B32 Model::initialize(const std::string& path, gfx::BackendRenderer* pRenderer)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    ASSERT(ret);

    loadTextures(&model, pRenderer);
    m_materials = loadMaterials(&model);
    m_submeshes = loadMeshes(&model, pRenderer, &m_vertBufferId, &m_indBufferId);
    return false;
}


B32 Model::cleanUp()
{
    return false;
}
} // jcl