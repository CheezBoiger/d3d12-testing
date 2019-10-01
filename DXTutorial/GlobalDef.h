#pragma once

#include "Renderer.h"

#include "Math/Vector4.h"
#include "Math/Matrix44.h"

namespace jcl {


using namespace m;

struct Globals {
  Vector4 _cameraPos;
  Matrix44 _viewToWorld;
  Matrix44 _worldToView;
};
} // jcl