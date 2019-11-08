//
#pragma once

#include "Math/Vector4.h"
#include "Math/Matrix44.h"




namespace jcl {


class Transform
{
public:
    Transform() { }
    
    

private:
    m::Vector4 m_up;
    m::Vector4 m_right;
    m::Vector4 m_front;
    
    m::Vector4 m_position;
    m::Vector4 m_rotation;

    m::Matrix44 m_inverseModelToWorld;
    m::Matrix44 m_modelToWorld;
};
} // jcl