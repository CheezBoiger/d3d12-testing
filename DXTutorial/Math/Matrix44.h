#pragma once

#include "../WinConfigs.h"
#include "Vector4.h"


namespace m {


struct Matrix33 {
  R32 _[3][3];

  R32 determinant() const {
    return  _[0][0] * (_[1][1] * _[2][2] - _[1][2] * _[2][1]) -
            _[0][1] * (_[1][0] * _[2][2] - _[1][2] * _[2][0]) +
            _[0][2] * (_[1][0] * _[2][1] - _[1][1] * _[2][0]);
  }

  R32* operator[](U32 r) { return _[r]; }
};


struct Matrix44 {
    R32 _[4][4];
    Matrix44(R32 a00 = 1.0f, R32 a01 = 0.0f, R32 a02 = 0.0f, R32 a03 = 0.0f,
             R32 a10 = 0.0f, R32 a11 = 1.0f, R32 a12 = 0.0f, R32 a13 = 0.0f,
             R32 a20 = 0.0f, R32 a21 = 0.0f, R32 a22 = 1.0f, R32 a23 = 0.0f,
             R32 a30 = 0.0f, R32 a31 = 0.0f, R32 a32 = 0.0f, R32 a33 = 1.0f)
    {
      _[0][0] = a00; _[0][1] = a01; _[0][2] = a02; _[0][3] = a03;
      _[1][0] = a10; _[1][1] = a11; _[1][2] = a12; _[1][3] = a13;
      _[2][0] = a20; _[2][1] = a21; _[2][2] = a22; _[2][3] = a23;
      _[3][0] = a30; _[3][1] = a31; _[3][2] = a32; _[3][3] = a33;
    }

    Matrix44 operator*(const Matrix44& other) const {
      R32 m00 = _[0][0];
      R32 m01 = _[0][1];
      R32 m02 = _[0][2];
      R32 m03 = _[0][3];
      R32 m10 = _[1][0];
      R32 m11 = _[1][1];
      R32 m12 = _[1][2];
      R32 m13 = _[1][3];
      R32 m20 = _[2][0];
      R32 m21 = _[2][1];
      R32 m22 = _[2][2];
      R32 m23 = _[2][3];
      R32 m30 = _[3][0];
      R32 m31 = _[3][1];
      R32 m32 = _[3][2];
      R32 m33 = _[3][3];
      R32 om00 = other._[0][0];
      R32 om01 = other._[0][1];
      R32 om02 = other._[0][2];
      R32 om03 = other._[0][3];
      R32 om10 = other._[1][0];
      R32 om11 = other._[1][1];
      R32 om12 = other._[1][2];
      R32 om13 = other._[1][3];
      R32 om20 = other._[2][0];
      R32 om21 = other._[2][1];
      R32 om22 = other._[2][2];
      R32 om23 = other._[2][3];
      R32 om30 = other._[3][0];
      R32 om31 = other._[3][1];
      R32 om32 = other._[3][2];
      R32 om33 = other._[3][3]; 
      return Matrix44( 
        m00 * om00 + m01 * om10 + m02 * om20 + m03 * om30,
        m00 * om01 + m01 * om11 + m02 * om21 + m03 * om31,
        m00 * om02 + m01 * om12 + m02 * om22 + m03 * om32,
        m00 * om03 + m01 * om13 + m02 * om23 + m03 * om33,

        m10 * om00 + m11 * om10 + m12 * om20 + m13 * om30,
        m10 * om01 + m11 * om11 + m12 * om21 + m13 * om31,
        m10 * om02 + m11 * om12 + m12 * om22 + m13 * om32,
        m10 * om03 + m11 * om13 + m12 * om23 + m13 * om33,

        m20 * om00 + m21 * om10 + m22 * om20 + m23 * om30,
        m20 * om01 + m21 * om11 + m22 * om21 + m23 * om31,
        m20 * om02 + m21 * om12 + m22 * om22 + m23 * om32,
        m20 * om03 + m21 * om13 + m22 * om23 + m23 * om33,

        m30 * om00 + m31 * om10 + m32 * om20 + m33 * om30,
        m30 * om01 + m31 * om11 + m32 * om21 + m33 * om31,
        m30 * om02 + m31 * om12 + m32 * om22 + m33 * om32,
        m30 * om03 + m31 * om13 + m32 * om23 + m33 * om33
      );
    }

    Matrix44 operator*(R32 scalar) const {
      return Matrix44(
          _[0][0] * scalar, _[0][1] * scalar,
          _[0][2] * scalar, _[0][3] * scalar,
          _[1][0] * scalar, _[1][1] * scalar,
          _[1][2] * scalar, _[1][3] * scalar,
          _[2][0] * scalar, _[2][1] * scalar,
          _[2][2] * scalar, _[2][3] * scalar,
          _[3][0] * scalar, _[3][1] * scalar,
          _[3][2] * scalar, _[3][3] * scalar
      );
    }

    Matrix44 operator+(const Matrix44& other) const {
      return Matrix44(
          _[0][0] + other._[0][0], _[0][1] + other._[0][1],
          _[0][2] + other._[0][2], _[0][3] + other._[0][3],
          _[1][0] + other._[1][0], _[1][1] + other._[1][1],
          _[1][2] + other._[1][2], _[1][3] + other._[1][3],
          _[2][0] + other._[2][0], _[2][1] + other._[2][1],
          _[2][2] + other._[2][2], _[2][3] + other._[2][3],
          _[3][0] + other._[3][0], _[3][1] + other._[3][1],
          _[3][2] + other._[3][2], _[3][3] + other._[3][3]
      );
    }

    Matrix44 operator-(const Matrix44& other) const {
      return Matrix44(
          _[0][0] - other._[0][0], _[0][1] - other._[0][1],
          _[0][2] - other._[0][2], _[0][3] - other._[0][3],
          _[1][0] - other._[1][0], _[1][1] - other._[1][1],
          _[1][2] - other._[1][2], _[1][3] - other._[1][3],
          _[2][0] - other._[2][0], _[2][1] - other._[2][1],
          _[2][2] - other._[2][2], _[2][3] - other._[2][3],
          _[3][0] - other._[3][0], _[3][1] - other._[3][1],
          _[3][2] - other._[3][2], _[3][3] - other._[3][3]
      );
    }

    Matrix44 operator/(const Matrix44& other) const {
      return Matrix44(
          _[0][0] / other._[0][0], _[0][1] / other._[0][1],
          _[0][2] / other._[0][2], _[0][3] / other._[0][3],
          _[1][0] / other._[1][0], _[1][1] / other._[1][1],
          _[1][2] / other._[1][2], _[1][3] / other._[1][3],
          _[2][0] / other._[2][0], _[2][1] / other._[2][1],
          _[2][2] / other._[2][2], _[2][3] / other._[2][3],
          _[3][0] / other._[3][0], _[3][1] / other._[3][1],
          _[3][2] / other._[3][2], _[3][3] / other._[3][3]
      );
    }
    
    Matrix33 minor(U32 row, U32 col) const {
        Matrix33 minor;
        U32 r = 0, c;
        for (U32 i = 0; i < 4; ++i) {
          if (i == row) continue;
          c = 0;
          for (U32 j = 0; j < 4; ++j) {
            if (j == col) continue;
            minor[r][c] = _[i][j];
            c++;
          }
          r++;
        }
        return minor;
    }

    Matrix44 inverse() const {
      R32 detA = determinant();
      if (detA == 0.0f) {
        return Matrix44();
      }
      Matrix44 inverse = adjugate() * (1.0f / detA);
      return inverse;
    }


    Matrix44 adjugate() const {
      // Calculating our adjugate using the transpose of the cofactor of our
      // matrix.
      Matrix44 CofactorMatrix;
      R32 sign = 1.0f;
      for (U32 row = 0; row < 4; ++row) {
        sign = -sign;
        for (U32 col = 0; col < 4; ++col) {
          sign = -sign;
          CofactorMatrix[row][col] = minor(row, col).determinant() * sign;
        }
      }
      // Transpose this CofactorMatrix to get the adjugate.
      return CofactorMatrix.transpose();
    }

    Matrix44 transpose() const {
      return Matrix44(
        _[0][0], _[1][0], _[2][0], _[3][0],
        _[0][1], _[1][1], _[2][1], _[3][1],
        _[0][2], _[1][2], _[2][2], _[3][2],
        _[0][3], _[1][3], _[2][3], _[3][3]
      );
    }


    R32 determinant() const {
  return  _[0][0] * (_[1][1] * (_[2][2] * _[3][3] - _[2][3] * _[3][2]) -
                        _[1][2] * (_[2][1] * _[3][3] - _[2][3] * _[3][1]) +
                        _[1][3] * (_[2][1] * _[3][2] - _[2][2] * _[3][1])
                    ) -
          _[0][1] * (_[1][0] * (_[2][2] * _[3][3] - _[2][3] * _[3][2]) -
                        _[1][2] * (_[2][0] * _[3][3] - _[2][3] * _[3][0]) +
                        _[1][3] * (_[2][0] * _[3][2] - _[2][2] * _[3][0])
                    ) +
          _[0][2] * (_[1][0] * (_[2][1] * _[3][3] - _[2][3] * _[3][1]) -
                        _[1][1] * (_[2][0] * _[3][3] - _[2][3] * _[3][0]) +
                        _[1][3] * (_[2][0] * _[3][1] - _[2][1] * _[3][0])
                    ) -
          _[0][3] * (_[1][0] * (_[2][1] * _[3][2] - _[2][2] * _[3][1]) -
                        _[1][1] * (_[2][0] * _[3][2] - _[2][2] * _[3][0]) +
                        _[1][2] * (_[2][0] * _[3][1] - _[2][1] * _[3][0]) );
    }

    R32* operator[](U32 i) { return _[i]; }

    static Matrix44 perspectiveRH(R32 fovy, R32 aspect, R32 zNear, R32 zFar) {
      R32 tanHalfFov = tanf(fovy * 0.5f);
      Matrix44 per;
      per[0][0] = 1.0f / (aspect * tanHalfFov);
      per[1][1] = 1.0f / tanHalfFov;
      per[2][2] = zFar / (zNear - zFar);
      per[3][2] = zNear * zFar / (zNear - zFar);
      per[2][3] = -1.0f;
      per[3][3] = 0.0f;
      return per;
    }

    static Matrix44 orthographicRH(R32 width, R32 height, R32 zNear, R32 zFar);
    static Matrix44 translate(const Matrix44& mat, const Vector4& translation);
    static Matrix44 scale(const Matrix44& mat, const Vector4& sc);
    static Matrix44 rotate(const Matrix44& mat);

    static Matrix44 lookAtRH(const Vector3& position, const Vector3& target, const Vector3& up) {
      Vector3 zaxis = (position - target).normalize();
      Vector3 xaxis = up.cross(zaxis).normalize();
      Vector3 yaxis = zaxis.cross(xaxis);
    
      Matrix44 view(
        xaxis._x,              yaxis._x,             zaxis._x,            0.0f,
        xaxis._y,              yaxis._y,             zaxis._y,            0.0f,
        xaxis._z,              yaxis._z,             zaxis._z,            0.0f,
        -xaxis.dot(position), -yaxis.dot(position), -zaxis.dot(position), 1.0f
      );

      return view;
    }
};
} // m