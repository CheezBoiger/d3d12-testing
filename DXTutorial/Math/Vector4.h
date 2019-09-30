#pragma once

#include "../WinConfigs.h"


namespace m {


struct Vector4 {
  union { struct { R32 _x, _y, _z, _w; };
          struct { R32 _r, _g, _b, _a; }; };
  Vector4(R32 x = 0.0f, R32 y = 0.0f, R32 z = 0.0f, R32 w = 1.0f) { }
};
} // m