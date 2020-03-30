
#include "Matrix44.h"


namespace m {



Matrix44 Matrix44::perspectiveRH(R32 fovy, R32 aspect, R32 zNear, R32 zFar) {
    R32 tanHalfFov = tanf(fovy * 0.5f);
    Matrix44 per;
    per[0][0] = 1.0f / (aspect * tanHalfFov);
    per[1][1] = 1.0f / tanHalfFov;
    per[2][2] = zNear / (zFar - zNear);
    per[3][2] = zFar * zNear / (zFar - zNear);
    per[2][3] = -1.0f;
    per[3][3] = 0.0f;
    return per;
}


Matrix44 Matrix44::orthographicRH(R32 width, R32 height, R32 zNear, R32 zFar)
{
    return Matrix44(2.0f/width,    0,          0,                  0,
                    0,          2.0f/height,   0,                  0,
                    0,          0,          1.0f/(zNear-zFar),     0,
                    0,          0,          zNear/(zNear-zFar), 1.0f);
}
}