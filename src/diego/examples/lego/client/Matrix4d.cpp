#include "Matrix4d.h"

#include "Rotation.h"

#include <iostream>

MfMatrix4d &
MfMatrix4d::SetTranslation(const MfVec3d &t)
{
    _data[0][0] = 1.0;  _data[0][1] = 0.0;  _data[0][2] = 0.0;  _data[0][3] = 0.0;
    _data[1][0] = 0.0;  _data[1][1] = 1.0;  _data[1][2] = 0.0;  _data[1][3] = 0.0;
    _data[2][0] = 0.0;  _data[2][1] = 0.0;  _data[2][2] = 1.0;  _data[2][3] = 0.0;
    _data[3][0] = t[0]; _data[3][1] = t[1]; _data[3][2] = t[2]; _data[3][3] = 1.0;

    return *this;
}

MfMatrix4d &
MfMatrix4d::SetScale(const MfVec3d &scale)
{
    _data[0][0] = scale[0]; _data[0][1] = 0.0;      _data[0][2] = 0.0;      _data[0][3] = 0.0;
    _data[1][0] = 0.0;      _data[1][1] = scale[1]; _data[1][2] = 0.0;      _data[1][3] = 0.0;
    _data[2][0] = 0.0;      _data[2][1] = 0.0;      _data[2][2] = scale[2]; _data[2][3] = 0.0;
    _data[3][0] = 0.0;      _data[3][1] = 0.0;      _data[3][2] = 0.0;      _data[3][3] = 1.0;

    return *this;
}

MfMatrix4d &
MfMatrix4d::SetRotation(const MfRotation &rot)
{
    MfVec3d i;
    double r;
    rot.GetQuaternion(&i[0], &i[1], &i[2], &r);

    _data[0][0] = 1.0 - 2.0 * (i[1] * i[1] + i[2] * i[2]);
    _data[0][1] =       2.0 * (i[0] * i[1] + i[2] *    r);
    _data[0][2] =       2.0 * (i[2] * i[0] - i[1] *    r);
    _data[0][3] = 0.0;

    _data[1][0] =       2.0 * (i[0] * i[1] - i[2] *    r);
    _data[1][1] = 1.0 - 2.0 * (i[2] * i[2] + i[0] * i[0]);
    _data[1][2] =       2.0 * (i[1] * i[2] + i[0] *    r);
    _data[1][3] = 0.0;

    _data[2][0] =       2.0 * (i[2] * i[0] + i[1] *    r);
    _data[2][1] =       2.0 * (i[1] * i[2] - i[0] *    r);
    _data[2][2] = 1.0 - 2.0 * (i[1] * i[1] + i[0] * i[0]);
    _data[2][3] = 0.0;

    _data[3][0] = 0.0;
    _data[3][1] = 0.0;
    _data[3][2] = 0.0;
    _data[3][3] = 1.0;

    return *this;
}

MfMatrix4d
MfMatrix4d::GetTranspose() const
{
    MfMatrix4d transpose;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            transpose._data[j][i] = _data[i][j];
        }
    }

    return transpose;
}

MfMatrix4d
MfMatrix4d::GetInverse() const
{
    // Based on Open Inventor's SbMatrix::inverse() which was based on
    // code from Numerical Recipies in C.
    int index[4];
    double d, invmat[4][4], temp;
    MfMatrix4d inverse = *this;

    inverse._LUDecomposition(index, d);
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            invmat[j][i] = 0.0;
        }
        invmat[j][j] = 1.0;
        inverse._LUBackSubstitution(index, invmat[j]);
    }

    // transpose invmat
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < j; i++) {
            temp = invmat[i][j];
            invmat[i][j] = invmat[j][i];
            invmat[j][i] = temp;
        }
    }

    inverse.Set(invmat);

    return inverse;
}

void
MfMatrix4d::_LUDecomposition(int index[4], double &d)
{
    int imax = 0;
    double big, dum, sum, temp;
    double vv[4];

    d = 1.0;
    
    for (int i = 0; i < 4; i++) {
        big = 0.0;
        for (int j = 0; j < 4; j++) {
            if ((temp = std::abs(_data[i][j])) > big) {
                big = temp;
            }
        }
        if (big == 0.0) {
	    _data[i][i] = 1e-6;
	    big = _data[i][i];
        }
        vv[i] = 1.0 / big;
    }

    for (int j = 0; j < 4; j++) {
	// BLOCK 1
        for(int i = 0; i < j; i++) {
            sum = _data[i][j];
            for(int k = 0; k < i; k++)
                sum -= _data[i][k] * _data[k][j];
            _data[i][j] = sum;
        }

        big = 0.0;

	// BLOCK 2
        for (int i = j; i < 4; i++) {
            sum = _data[i][j];
            for(int k = 0; k < j; k++)
                sum -= _data[i][k] * _data[k][j];
            _data[i][j] = sum;
            if((dum = vv[i] * std::abs(sum)) >= big) {
                big = dum;
                imax = i;
            }
        }

	// BLOCK 3
        if (j != imax) {
            for(int k = 0; k < 4; k++) {
                dum = _data[imax][k];
                _data[imax][k] = _data[j][k];
                _data[j][k] = dum;
            }
            d = -d;
            vv[imax] = vv[j];
        }

	// BLOCK 4
        index[j] = imax;
        if(_data[j][j] == 0.0) _data[j][j] = 1e-20;
	
	// BLOCK 5
        if(j != 4 - 1) {
            dum = 1.0 / (_data[j][j]);
            for(int i = j + 1; i < 4; i++)
                _data[i][j] *= dum;
        }
    }
}

void
MfMatrix4d::_LUBackSubstitution(int index[4], double b[4]) const
{
    int ii = -1, ip;
    double sum;

    for (int i = 0; i < 4; i++) {
        ip = index[i];
        sum = b[ip];
        b[ip] = b[i];
        if (ii >= 0) {
            for (int j = ii; j <= i - 1; j++) {
                sum -= _data[i][j] * b[j];
            }
        } else if (sum) {
            ii = i;
        }
        b[i] = sum;
    }

    for (int i = 4 - 1; i >= 0; i--) {
        sum = b[i];
        for (int j = i + 1; j < 4; j++) {
            sum -= _data[i][j]*b[j];
        }
        b[i] = sum / _data[i][i];
    }
}

double
MfMatrix4d::GetDeterminant() const
{
    return 
        _data[0][3] * _GetDeterminant3(1, 2, 3, 0, 1, 2) +
        _data[1][3] * _GetDeterminant3(0, 2, 3, 0, 1, 2) +
        _data[2][3] * _GetDeterminant3(0, 1, 3, 0, 1, 2) +
        _data[3][3] * _GetDeterminant3(0, 1, 2, 0, 1, 2);
}

double
MfMatrix4d::GetDeterminant3() const
{
    return _GetDeterminant3(0, 1, 2, 0, 1, 2);
}

double
MfMatrix4d::_GetDeterminant3(size_t row1, size_t row2, size_t row3,
                             size_t col1, size_t col2, size_t col3) const
{
    return 
        _data[row1][col1] * _data[row2][col2] * _data[row3][col3] +
        _data[row1][col2] * _data[row2][col3] * _data[row3][col1] +
        _data[row1][col3] * _data[row2][col1] * _data[row3][col2] -
        _data[row1][col1] * _data[row2][col3] * _data[row3][col2] -
        _data[row1][col2] * _data[row2][col1] * _data[row3][col3] -
        _data[row1][col3] * _data[row2][col2] * _data[row3][col1];
}

MfVec3d
MfMatrix4d::Transform(const MfVec3d &v) const
{
    const MfMatrix4d &m = *this;

#define MF_MULT_VEC(i)                                  \
    v[0] * m._data[0][i] +                              \
    v[1] * m._data[1][i] +                              \
    v[2] * m._data[2][i] +                              \
           m._data[3][i]

    return MfVec3d(MF_MULT_VEC(0), MF_MULT_VEC(1), MF_MULT_VEC(2));

#undef MF_MULT_VEC
}

MfVec3d
MfMatrix4d::TransformDirection(const MfVec3d &dir) const
{
    const MfMatrix4d &m = *this;

#define MF_MULT_VEC(i)                                  \
    dir[0] * m._data[0][i] +                            \
    dir[1] * m._data[1][i] +                            \
    dir[2] * m._data[2][i]

    return MfVec3d(MF_MULT_VEC(0), MF_MULT_VEC(1), MF_MULT_VEC(2));

#undef MF_MULT_VEC
}

MfVec3f
MfMatrix4d::Transform(const MfVec3f &v) const
{
#define MF_MULT_VEC(i)                                      \
    float(v[0] * _data[0][i] +                              \
          v[1] * _data[1][i] +                              \
          v[2] * _data[2][i] +                              \
                 _data[3][i])

    return MfVec3f(MF_MULT_VEC(0), MF_MULT_VEC(1), MF_MULT_VEC(2));

#undef MF_MULT_VEC
}

MfVec3f
MfMatrix4d::TransformDirection(const MfVec3f &dir) const
{
#define MF_MULT_VEC(i)                                      \
    float(dir[0] * _data[0][i] +                            \
          dir[1] * _data[1][i] +                            \
          dir[2] * _data[2][i])

    return MfVec3f(MF_MULT_VEC(0), MF_MULT_VEC(1), MF_MULT_VEC(2));

#undef MF_MULT_VEC
}

MfMatrix4d &
MfMatrix4d::operator*=(const MfMatrix4d &m)
{
    MfMatrix4d tmp = *this;

#define MF_MULT_MAT(i, j)                       \
    _data[i][j] =                               \
        (tmp._data[i][0] * m._data[0][j] +      \
         tmp._data[i][1] * m._data[1][j] +      \
         tmp._data[i][2] * m._data[2][j] +      \
         tmp._data[i][3] * m._data[3][j]) 

    MF_MULT_MAT(0, 0);
    MF_MULT_MAT(0, 1);
    MF_MULT_MAT(0, 2);
    MF_MULT_MAT(0, 3);
    MF_MULT_MAT(1, 0);
    MF_MULT_MAT(1, 1);
    MF_MULT_MAT(1, 2);
    MF_MULT_MAT(1, 3);
    MF_MULT_MAT(2, 0);
    MF_MULT_MAT(2, 1);
    MF_MULT_MAT(2, 2);
    MF_MULT_MAT(2, 3);
    MF_MULT_MAT(3, 0);
    MF_MULT_MAT(3, 1);
    MF_MULT_MAT(3, 2);
    MF_MULT_MAT(3, 3);

#undef MF_MULT_MAT

    return *this;
}

MfVec4d
operator*(const MfVec4d &vec, const MfMatrix4d &m)
{
#define MF_MULT_MAT(col)                        \
    (vec[0] * m._data[0][col] +	                \
     vec[1] * m._data[1][col] +                 \
     vec[2] * m._data[2][col] +                 \
     vec[3] * m._data[3][col])

    return MfVec4d(MF_MULT_MAT(0),
                   MF_MULT_MAT(1),
                   MF_MULT_MAT(2),
                   MF_MULT_MAT(3));

#undef MF_MULT_MAT
}

MfVec4d
operator*(const MfMatrix4d &m, const MfVec4d &vec)
{
#define MF_MULT_MAT(row)                        \
    (vec[0] * m._data[row][0] +                 \
     vec[1] * m._data[row][1] +                 \
     vec[2] * m._data[row][2] +                 \
     vec[3] * m._data[row][3])

    return MfVec4d(MF_MULT_MAT(0),
                   MF_MULT_MAT(1),
                   MF_MULT_MAT(2),
                   MF_MULT_MAT(3));

#undef MF_MULT_MAT
}

MfVec4f
operator*(const MfVec4f &vec, const MfMatrix4d &m)
{
#define MF_MULT_MAT(col)                        \
    float(vec[0] * m._data[0][col] +            \
          vec[1] * m._data[1][col] +            \
          vec[2] * m._data[2][col] +            \
          vec[3] * m._data[3][col])

    return MfVec4f(MF_MULT_MAT(0),
		   MF_MULT_MAT(1),
		   MF_MULT_MAT(2),
		   MF_MULT_MAT(3));

#undef MF_MULT_MAT
}

MfVec4f
operator*(const MfMatrix4d &m, const MfVec4f &vec)
{
#define MF_MULT_MAT(row)                        \
    float(vec[0] * m._data[row][0] +            \
          vec[1] * m._data[row][1] +            \
          vec[2] * m._data[row][2] +            \
          vec[3] * m._data[row][3])

    return MfVec4f(MF_MULT_MAT(0),
		   MF_MULT_MAT(1),
		   MF_MULT_MAT(2),
		   MF_MULT_MAT(3));

#undef MF_MULT_MAT
}

std::ostream &
operator<<(std::ostream &os, const MfMatrix4d &mat) 
{
    os << "(";
    for (int i = 0; i < 4; ++i) {
        os << "(";
        os << mat._data[i][0] << ", " << mat._data[i][1] << ", ";
        os << mat._data[i][2] << ", " << mat._data[i][3] << ")";
        os << ")";
        if (i != 3) {
            os << ", ";
        }
    }
    os << ")";
    return os;
}
