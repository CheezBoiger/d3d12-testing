#pragma once

#include "../WinConfigs.h"
#include "Matrix44.h"
#include "Vector4.h"

namespace m {

struct Quaternion {
    Quaternion(R32 x = 0.0f, R32 y = 0.0f, R32 z = 0.0f, R32 w = 1.0f)
        : _x(x), _y(y), _z(z), _w(w) { }

    
    Quaternion operator+(const Quaternion& rh) const {
        return Quaternion(_x + rh._x, _y + rh._y, _z + rh._z, _w + rh._w);
    }

    Quaternion operator-(const Quaternion& rh) const {
        return Quaternion(_x - rh._x, _y - rh._y, _z - rh._z, _w - rh._w);
    }

    Quaternion operator*(const Quaternion& rh) const {
        return Quaternion(_x * rh._x, _y * rh._y, _z * rh._z, _w * rh._w);
    }

    Quaternion operator/(const Quaternion& rh) const {
        return Quaternion(_x / rh._x, _y / rh._y, _z / rh._z, _w / rh._w);
    }

    void operator+=(const Quaternion& rh) {
        _x += rh._x;
        _y += rh._y;
        _z += rh._z;
        _w += rh._w;
    }

    Quaternion conjugate() const {
        return Quaternion(-_x, -_y, -_z, _w);
    }

    B32 isUnit() const {
        return norm() == 1.0f;
    }

    R32 norm() const {
        return sqrtf(_x * _x + _y * _y + _z * _z + _w * _w);
    }

    Quaternion inverse() const {
        R32 norm2 = (_x * _x + _y * _y + _z * _z + _w * _w);
        return conjugate() / norm2;
    }

    Quaternion normalize() const {
        return (*this / norm());
    }

    operator Vector4() const {
        return Vector4(_x, _y, _z, _w);
    }

    R32 _x, _y, _z, _w;
};
} // m