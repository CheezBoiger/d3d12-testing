#pragma once

#include "Vector4.h"

namespace m {

struct Bounds3D
{
    Vector3 _min, _max;
    
    Bounds3D(const Vector3& mmin = Vector3(), const Vector3& mmax = Vector3())
        : _min(mmin), _max(mmax) { }

    Vector3 getExtent() const {
        return _max - _min;
    }

    Vector3 getCenter() const {
        Vector3 extentHalf = getExtent() * 0.5f;
        return _min + extentHalf;
    }
};
}