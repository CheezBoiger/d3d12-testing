#pragma once

#include "BackendRenderer.h"
#include "Math/Matrix44.h"
#include "Math/Vector4.h"

namespace jcl {


using namespace m;


class Camera {
public:
  
  void initialize();
  
  void update();



private:
  
  R32 m_fov;
  R32 m_zNear;
  R32 m_zFar;
  R32 m_aspect;

  // Projection matrix.
  Matrix44 m_viewToClip;
  // View matrix.
  Matrix44 m_worldToView;  
};
} // jcl