#pragma once

#include "Renderer.h"
#include "Math/Matrix44.h"
#include "Math/Vector4.h"

namespace jcl {


using namespace m;


class Camera {
public:

private:
  R32 m_fov;
  R32 m_zNear;
  R32 m_zFar;
  R32 m_aspect;

  Matrix44 m_projection;
  Matrix44 m_worldToView;  
};
} // jcl