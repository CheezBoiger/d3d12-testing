#pragma once

#include "BackendRenderer.h"

#include "Math/Vector4.h"
#include "Math/Matrix44.h"

namespace jcl {


using namespace m;

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
  // Inverse Projection.
  Matrix44 _clipToView;
  // target view size.
  U32 _targetSize[4]; 
};


struct PerMeshDescriptor
{
  // Model-View-Projection transform.
  Matrix44 _worldToViewClip;
  // Previous frame Model-View-Projection transform.
  Matrix44 _previousWorldToViewClip;
  // Normal Correction 
  Matrix44 _n;
  // Materials
  Vector4 _matrialFlags;
};


struct GBuffer {
  gfx::Resource* pAlbedoTexture;
  gfx::Resource* pMaterialTexture;
  gfx::Resource* pNormalTexture;
  gfx::Resource* pEmissiveTexture;
  gfx::Resource* pVelocityTexture;
};


void generateGBufferResources(GBuffer* pOut, gfx::BackendRenderer* backend);

} // jcl