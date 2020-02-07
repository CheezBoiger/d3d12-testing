#pragma once

#include "../WinConfigs.h"


namespace m {


struct Vector4;

struct Vector3 {
  union { struct { R32 _x, _y, _z; };
          struct { R32 _r, _g, _b; }; };
  Vector3(R32 x = 0.0f, R32 y = 0.0f, R32 z = 0.0f)
    : _x(x), _y(y), _z(z) { }

  Vector3 operator+(const Vector3& other) const { 
    return Vector3(_x + other._x, _y + other._y, _z + other._z);
  }

  Vector3 operator-(const Vector3& other) const {
    return Vector3(_x - other._x, _y - other._y, _z - other._z);
  }

  Vector3 operator*(const Vector3& other) const {
    return Vector3(_x * other._x, _y * other._y, _z * other._z);
  }

  Vector3 operator*(R32 scalar) const {
    return Vector3(_x * scalar, _y * scalar, _z * scalar);
  }

  Vector3 operator/(const Vector3& other) const {
    return Vector3(_x / other._x, _y / other._y, _z / other._z);
  }

  Vector3 operator/(R32 scalar) const {
    return Vector3(_x / scalar, _y / scalar, _z / scalar);
  }

  Vector3 operator-() const {
    return Vector3(-_x, -_y, -_z);
  }

  Vector3 cross(const Vector3& other) const {
    return Vector3(
      _y * other._z - _z * other._y,
      _z * other._x - _x * other._z,
      _x * other._y - _y * other._x
    );
  }

  R32 dot(const Vector3& other) const {
    return (_x * other._x + _y * other._y + _z * other._z);
  }

  B32 operator==(const Vector3& other) const {
    return (_x == other._x) && (_y == other._y) && (_z == other._z);
  }

  B32 operator!=(const Vector3& other) const {
    return !(*this == other);
  }

  R32& operator[](U32 i) { return (&_x)[ i ]; }

  R32 length() const {
    return sqrtf(_x * _x + _y * _y + _z * _z);
  }

  Vector3 normalize() const {
    R32 magnitude = length();
    return (*this) / magnitude;
  }
};


struct Vector4 {
  union { struct { R32 _x, _y, _z, _w; };
          struct { R32 _r, _g, _b, _a; }; };
  Vector4(R32 x = 0.0f, R32 y = 0.0f, R32 z = 0.0f, R32 w = 1.0f)
    : _x(x), _y(y), _z(z), _w(w) { }

  Vector4(const Vector3& v, R32 w = 1.0f)
    : _x(v._x), _y(v._y), _z(v._z), _w(w) { }

  R32& operator[](U32 i) { return (&_x)[ i ]; }


  operator Vector3 () { return Vector3(_x, _y, _z); }
};
} // m