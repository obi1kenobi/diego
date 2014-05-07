#ifndef __MF_MATRIX4D_H__
#define __MF_MATRIX4D_H__

#include "Vec3f.h"
#include "Vec3d.h"
#include "Vec4d.h"
#include "Vec4f.h"

#include <iosfwd>

class MfRotation;

/// An abstraction for a matrix of 4x4 \c double elements.
/// Matrices are specified in row-major order.
class MfMatrix4d {
  public:
    /// Default constructor. Leaves the matrix undefined.
    MfMatrix4d() {
    }

    /// Constructor that creates a matrix from sixteen values that
    /// enumerate each of the matrix values in row-major order.
    MfMatrix4d(double m00, double m01, double m02, double m03,
               double m10, double m11, double m12, double m13,
               double m20, double m21, double m22, double m23,
               double m30, double m31, double m32, double m33) {
        Set(m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33);
    }

    /// Constructor that creates a matrix from a 3x3 array of
    /// double values in row-major order.
    MfMatrix4d(const double m[4][4]) {
	Set(m);
    }

    /// Set matrix values from four double values specified in row-major
    /// order.
    MfMatrix4d & Set(double m00, double m01, double m02, double m03,
                     double m10, double m11, double m12, double m13,
                     double m20, double m21, double m22, double m23,
                     double m30, double m31, double m32, double m33) {
        _data[0][0] = m00; _data[0][1] = m01; _data[0][2] = m02; _data[0][3] = m03;
        _data[1][0] = m10; _data[1][1] = m11; _data[1][2] = m12; _data[1][3] = m13;
        _data[2][0] = m20; _data[2][1] = m21; _data[2][2] = m22; _data[2][3] = m23;
        _data[3][0] = m30; _data[3][1] = m31; _data[3][2] = m32; _data[3][3] = m33;
        return *this;
    }

    /// Set matrix values from a 4x4 array specified in row-major order.
    MfMatrix4d & Set(const double m[4][4]) {
        _data[0][0] = m[0][0]; _data[0][1] = m[0][1]; _data[0][2] = m[0][2]; _data[0][3] = m[0][3];
        _data[1][0] = m[1][0]; _data[1][1] = m[1][1]; _data[1][2] = m[1][2]; _data[1][3] = m[1][3];
        _data[2][0] = m[2][0]; _data[2][1] = m[2][1]; _data[2][2] = m[2][2]; _data[2][3] = m[2][3];
        _data[3][0] = m[3][0]; _data[3][1] = m[3][1]; _data[3][2] = m[3][2]; _data[3][3] = m[3][3];
        return *this;
    }

    /// Sets the matrix to the identity matrix.
    MfMatrix4d & SetIdentity() {
	return SetDiagonal(1.0);
    }

    /// Sets the matrix to zero.
    MfMatrix4d & SetZero() {
	return SetDiagonal(0.0);
    }

    /// Sets the matrix to \p scalar times the identity matrix.
    MfMatrix4d & SetDiagonal(double scalar) {
        _data[0][0] = scalar; _data[0][1] = 0.0;    _data[0][2] = 0.0;    _data[0][3] = 0.0;
        _data[1][0] = 0.0;    _data[1][1] = scalar; _data[1][2] = 0.0;    _data[1][3] = 0.0;
        _data[2][0] = 0.0;    _data[2][1] = 0.0;    _data[2][2] = scalar; _data[2][3] = 0.0;
        _data[3][0] = 0.0;    _data[3][1] = 0.0;    _data[3][2] = 0.0;    _data[3][3] = scalar;
        return *this;
    }

    /// Make matrix a translation matrix for given translation vector \p t.
    MfMatrix4d & SetTranslation(const MfVec3d &t);

    /// Make matrix a scale matrix for given scaling \p scale.
    MfMatrix4d & SetScale(const MfVec3d &scale);

    /// Make matrix a rotation matrix for given rotation \p rot.
    MfMatrix4d & SetRotation(const MfRotation &rot);

    /// Get the matrix elements copied over into provided 4x4 array in
    /// row-major order.
    void Get(double m[4][4]) const {
        m[0][0] = _data[0][0]; m[0][1] = _data[0][1]; m[0][2] = _data[0][2]; m[0][3] = _data[0][3];
        m[1][0] = _data[1][0]; m[1][1] = _data[1][1]; m[1][2] = _data[1][2]; m[1][3] = _data[1][3];
        m[2][0] = _data[2][0]; m[2][1] = _data[2][1]; m[2][2] = _data[2][2]; m[2][3] = _data[2][3];
        m[3][0] = _data[3][0]; m[3][1] = _data[3][1]; m[3][2] = _data[3][2]; m[3][3] = _data[3][3];
    }

    /// Returns vector components as array of doubles.
    const double * Get() const {
	return &_data[0][0];
    }

    /// Returns the transpose of this matrix.
    MfMatrix4d GetTranspose() const;

    /// Returns the inverse of this matrix. Results are undefined if the
    /// matrix is not invertible.
    MfMatrix4d GetInverse() const;

    /// Returns the determinant of the matrix.
    double GetDeterminant() const;

    /// Returns the upper 3x3 matrix determinant.
    double GetDeterminant3() const;

    // Transform given vector \p vec by this matrix. The "w" coordinate is
    // assumed to be 1.
    MfVec3d Transform(const MfVec3d &vec) const;

    // Transform given direction vector \p vec by this matrix. This means
    // the vector is treated as if the "w" coordinate is 0.
    MfVec3d TransformDirection(const MfVec3d &dir) const;

    // Transform given vector \p vec by this matrix. The "w" coordinate is
    // assumed to be 1.
    MfVec3f Transform(const MfVec3f &vec) const;

    // Transform given direction vector \p vec by this matrix. This means
    // the vector is treated as if the "w" coordinate is 0.
    MfVec3f TransformDirection(const MfVec3f &dir) const;

    /// Allow indexed access to underlying values. This operator returns
    /// the indexed row. The user can nest the brackets, i.e., if  "mat"
    /// is of type MfMatrix4d, the user can use mat[row][column] to acces the
    /// matrix value at the given row and column.
    double * operator[](int i) { 
        return &_data[i][0]; 
    }

    /// Allow const indexed access to underlying values. This operator returns
    /// the indexed row. The user can nest the brackets, i.e., if  "mat"
    /// is of type MfMatrix4d, the user can use mat[row][column] to acces the
    /// matrix value at the given row and column.
    const double * operator[](int i) const { 
        return &_data[i][0]; 
    }

    /// Equality operator; component-wise.
    bool operator==(const MfMatrix4d &m) const {
        return _data[0][0] == m._data[0][0] &&
               _data[0][1] == m._data[0][1] &&
               _data[0][2] == m._data[0][2] &&
               _data[0][3] == m._data[0][3] &&
               _data[1][0] == m._data[1][0] &&
               _data[1][1] == m._data[1][1] &&
               _data[1][2] == m._data[1][2] &&
               _data[1][3] == m._data[1][3] &&
               _data[2][0] == m._data[2][0] &&
               _data[2][1] == m._data[2][1] &&
               _data[2][2] == m._data[2][2] &&
               _data[2][3] == m._data[2][3] &&
               _data[3][0] == m._data[3][0] &&
               _data[3][1] == m._data[3][1] &&
               _data[3][2] == m._data[3][2] &&
               _data[3][3] == m._data[3][3];
    }

    /// Inequality operator; component-wise.
    bool operator!=(const MfMatrix4d &m) const {
	return !(*this == m);
    }

    /// Post-multiplies the given matrix \p m into this matrix.
    MfMatrix4d & operator*=(const MfMatrix4d &m);

    /// Matrix multiplication
    friend MfMatrix4d operator*(const MfMatrix4d &m1, const MfMatrix4d &m2) {
	MfMatrix4d m = m1;
	return m *= m2;
    }

    /// Matrix and scalar multiplication.
    MfMatrix4d & operator*=(double scalar) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                _data[i][j] *= scalar;
            }
        }
        return *this;
    }

    /// Matrix and scalar multiplication.
    friend MfMatrix4d operator*(const MfMatrix4d &m1, double d) {
	MfMatrix4d m = m1;
	return m *= d;
    }

    /// Matrix and scalar multiplication.
    friend MfMatrix4d operator*(double scalar, const MfMatrix4d &m) {
        return m * scalar; 
    }

    /// Matrix and matrix addition.
    MfMatrix4d & operator+=(const MfMatrix4d &m) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                _data[i][j] += m._data[i][j];
            }
        }
        return *this;
    }

    /// Matrix and matrix addition.
    friend MfMatrix4d operator+(const MfMatrix4d &m1, const MfMatrix4d &m2) {
	MfMatrix4d m = m1;
	return m += m2;
    }

    /// Matrix and matrix subtraction.
    MfMatrix4d & operator-=(const MfMatrix4d &m) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                _data[i][j] -= m._data[i][j];
            }
        }
        return *this;
    }

    /// Matrix and matrix subtraction.
    friend MfMatrix4d operator-(const MfMatrix4d &m1, const MfMatrix4d &m2) {
	MfMatrix4d m = m1;
	return m -= m2;
    }

    /// Unary negation
    friend MfMatrix4d operator-(const MfMatrix4d &m) {
	return MfMatrix4d(-m._data[0][0], -m._data[0][1], -m._data[0][2], -m._data[0][3],
                          -m._data[1][0], -m._data[1][1], -m._data[1][2], -m._data[1][3],
                          -m._data[2][0], -m._data[2][1], -m._data[2][2], -m._data[2][3],
                          -m._data[3][0], -m._data[3][1], -m._data[3][2], -m._data[3][3]);
    }

    /// Divide matrix \p m1 by matrix \p m2
    friend MfMatrix4d operator/(const MfMatrix4d &m1, const MfMatrix4d &m2) {
	return m1 * m2.GetInverse();
    }

    /// Multiplies matrix \p m and column vector \p vec.
    friend MfVec4d operator*(const MfMatrix4d &m, const MfVec4d &vec);

    /// Multiplies row vector \p vec and matrix \p m.
    friend MfVec4d operator*(const MfVec4d &vec, const MfMatrix4d &m);

    /// Multiplies matrix \p m and floating point column vector \p vec.
    friend MfVec4f operator*(const MfMatrix4d &m, const MfVec4f &vec);

    /// Multiplies floating point row vector \p vec and matrix \p m.
    friend MfVec4f operator*(const MfVec4f &vec, const MfMatrix4d &m);

    /// Output vector to given stream \p os. To be used strictly for
    /// debugging or non-formal presentation of data since there is no
    /// guarantee that the formatting of the data won't change.
    friend std::ostream & operator<<(std::ostream &os, const MfMatrix4d &mat);

  private:
    void _LUDecomposition(int index[4], double &d);
    void _LUBackSubstitution(int index[4], double b[4]) const;
    double _GetDeterminant3(size_t row1, size_t row2, size_t row3,
			    size_t col1, size_t col2, size_t col3) const;

    double _data[4][4];
};

#endif // __MF_MATRIX4D_H__
