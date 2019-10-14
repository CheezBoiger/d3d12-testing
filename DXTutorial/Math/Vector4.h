#pragma once

#include "../WinConfigs.h"


namespace m {


struct Vector3 {
  union { struct { R32 _x, _y, _z; };
          struct { R32 _r, _g, _b; }; };
  Vector3(R32 x = 0.0f, R32 y = 0.0f, R32 z = 0.0f)
    : _x(x), _y(y), _z(z) { }

  Vector3 operator+(const Vector3& other) const;
  Vector3 operator-(const Vector3& other) const;
  Vector3 operator*(const Vector3& other) const;
  Vector3 operator/(const Vector3& other) const;

  Vector3 cross(const Vector3& other) const;
  R32 dot(const Vector3& other) const;

  R32& operator[](U32 i) { return (&_x)[ i ]; }
};


struct Vector4 {
  union { struct { R32 _x, _y, _z, _w; };
          struct { R32 _r, _g, _b, _a; }; };
  Vector4(R32 x = 0.0f, R32 y = 0.0f, R32 z = 0.0f, R32 w = 1.0f)
    : _x(x), _y(y), _z(z), _w(w) { }

  R32& operator[](U32 i) { return (&_x)[ i ];}
};
} // m