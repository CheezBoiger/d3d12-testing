#pragma once

#include "BackendRenderer.h"

#include "Math/Vector4.h"
#include "Math/Matrix44.h"
#include "Math/Bounds3D.h"

#include "Shaders/CommonShaderParams.h"

#include <vector>

#define GLOBAL_CONST_SLOT 0
#define MESH_TRANSFORM_SLOT 1
#define MATERIAL_DEF_SLOT 2

namespace jcl {


using namespace m;

struct Vertex
{
    struct { R32 _x, _y, _z, _w; } _position;
    struct { R32 _x, _y, _z, _w; } _normal;
    struct { R32 _x, _y, _z, _w; } _tangent;
    struct { R32 _x, _y, _z, _w; } _texcoords;
};

typedef U64 RenderUUID;

struct VertexBuffer
{
    RenderUUID resource;
    RenderUUID vertexBufferView;
};


struct IndexBuffer
{
    RenderUUID resource;
    RenderUUID indexBufferView;
};

// Global descriptor, to be used throughout each graphics render pass.
// Keep in mind that buffers in gpu MUST ALWAYS BE ALIGNED 16 BYTES (multiple of 16 bytes).
struct Globals 
{
    // Camera Position in world space.
    Vector4 _cameraPos;
    // Inverse View. View To World
    Matrix44 _viewToWorld;
    // View
    Matrix44 _worldToView;
    // Projection
    Matrix44 _proj;
    // View-Projection.
    Matrix44 _viewToClip;
    // Inverse View-Projection.
    Matrix44 _clipToView;
    // target view size.
    U32 _targetSize[4]; 
    // Allow bump mapping texturing.
    U32 _allowBumpMapping;
    // Near Z Plane.
    R32 _near;
    // Far Z Plane.
    R32 _far;
    // Sunlight shadow index that corresponds to the direction light array.
    I32 _sunlightShadowIndex;
};


struct PerMeshDescriptor
{
    // Model World transform.
    Matrix44 _world;
    // Model-View-Projection transform.
    Matrix44 _worldToViewClip;
    // Previous frame Model-View-Projection transform.
    Matrix44 _previousWorldToViewClip;
    // Normal Correction 
    Matrix44 _n;
};

struct PerMaterialDescriptor
{
    Vector4 _albedo;
    // Spec/Gloss factor if material flags is set to use.
    Vector4 _roughnessMetallicFactor;
    Vector4 _emissionFactor;
    Vector4 _albedoFactor;
    Vector4 _fresnelFactor;
    // Material flags.
    U32 _matrialFlags;
    U32 _pad0[3];
};


struct PerLightSpaceDescriptor
{
    Matrix44 _viewToClip;
};


// Deferred rendering textures, to be resolved later on.
struct GBuffer 
{
    gfx::Resource* pAlbedoTexture;
    gfx::Resource* pMaterialTexture;
    gfx::Resource* pNormalTexture;
    gfx::Resource* pEmissiveTexture;

    gfx::RenderTargetView* pAlbedoRTV;
    gfx::RenderTargetView* pMaterialRTV;
    gfx::RenderTargetView* pNormalRTV;
    gfx::RenderTargetView* pEmissiveRTV;

    gfx::ShaderResourceView* pAlbedoSRV;
    gfx::ShaderResourceView* pMaterialSRV;
    gfx::ShaderResourceView* pNormalSRV;
    gfx::ShaderResourceView* pEmissiveSRV;

    gfx::RenderPass* pRenderPass;
};

struct GeometryMaterialMap
{
    RenderUUID _albedoMap;
    RenderUUID _normalMap;
    RenderUUID _roughnessMetallicMap;
    RenderUUID _emissiveMap;
};

// Geometry Mesh describes the whole mesh, and the submeshes it is 
// composed of.
struct GeometryMesh
 {
    RenderUUID _meshTransform;
    RenderUUID _vertexBufferView;
    RenderUUID _indexBufferView;
    PerMeshDescriptor* _meshDescriptor;
    U32 _submeshCount;
    U32 _materialMapCount;
    GeometryMaterialMap* _materialMaps;
    Bounds3D _bounds;
};

// Geometry Submesh describes only the partial vertices that make up a 
// given primitive of the mesh. A group of submeshes can compose a mesh.
struct GeometrySubMesh
{
    RenderUUID _materialDescriptor;
    PerMaterialDescriptor* _matData;
    U32 _materialMapIdx;
    U32 _vertCount;
    U32 _vertInst;
    U32 _startVert;
    U32 _indCount;
    U32 _indOffset;
};

struct RenderGroup 
{
    // Render Targets.
    std::vector<gfx::RenderTargetView*> renderTargetViews;
    // Depth Stencil Target.
    gfx::DepthStencilView* _depthStencilView;
    // The given pipeline used for this render group.
    RenderUUID _pipeline;
    // Render in deferred lighting flow.
    B32 _isDeferred;
    // All render commands to be processed.
    GeometryMesh* _geometryMeshes;
    U32 meshCount;
};



void retrieveShader(const std::string& filepath, void** bytecode, size_t& length);
} // jcl