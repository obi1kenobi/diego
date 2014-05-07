#ifndef __MF_VEC3I_H__
#define __MF_VEC3I_H__

#include "IntTypes.h"

#include <cstddef>
#include <cmath>
#include <iosfwd>

///
/// Vector of 3 components of type int.
///
class MfVec3d;
class MfVec3f;

class MfVec3i {
  public:
    /// Default constructor makes an undefined vector.
    MfVec3i() {}

    /// A constructor that initializes the vector from a int array
    explicit MfVec3i(const int varray[3]) { 
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
    }

    /// A constructor that initializes the vector from the passed values.
    MfVec3i(int v0, int v1, int v2) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
    }

    /// A constructor that initializes the vector by smearing the given
    /// value along all vector components.
    explicit MfVec3i(int smearValue) {
        _data[0] = smearValue; 
        _data[1] = smearValue; 
        _data[2] = smearValue; 
    }

    /// Set vector values from given array.
    MfVec3i & Set(const int varray[3]) {
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
	return *this;
    }

    /// Sets vector values from given individual element values.
    MfVec3i & Set(int v0, int v1, int v2) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
	return *this;
    }

    /// A constructor that converts a float to int vector.
    MfVec3i(const MfVec3f &vfloat);

    /// A constructor that converts an double to int vector.
    MfVec3i(const MfVec3d &vdouble);

    /// Returns vector as an array of int values.
    const int * Get() const { 
        return _data; 
    }

    /// Allow indexed access to underlying values.
    int & operator[](size_t i) {
        return _data[i]; 
    }

    /// Allow indexed access to underlying const values.
    const int & operator[](size_t i) const { 
        return _data[i]; 
    }

    /// Assignment operator.
    MfVec3i & operator=(const MfVec3i &v) {
	_data[0] = v[0]; 
	_data[1] = v[1]; 
	_data[2] = v[2]; 
	return *this;
    }

    /// Equality operator.
    bool operator==(const MfVec3i &v) const {
	return 
            _data[0] == v._data[0] && 
            _data[1] == v._data[1] && 
            _data[2] == v._data[2];;
    }

    /// Inequality operator.
    bool operator!=(const MfVec3i &v) const {
	return ! (*this == v);
    }

    /// Addition operator.
    friend MfVec3i operator+(const MfVec3i &v1, const MfVec3i &v2) {
	return MfVec3i(v1._data[0] + v2._data[0], 
                       v1._data[1] + v2._data[1],
                       v1._data[2] + v2._data[2]);
    }

    /// Subtraction operator.
    friend MfVec3i operator-(const MfVec3i &v1, const MfVec3i &v2) {
	return MfVec3i(v1._data[0] - v2._data[0], 
                       v1._data[1] - v2._data[1],
                       v1._data[2] - v2._data[2]);
    }

    /// Multiplication by a scalar operator.
    friend MfVec3i operator*(const MfVec3i &v, int scalar) {
	return MfVec3i(int(v._data[0] * scalar), 
                       int(v._data[1] * scalar),
                       int(v._data[2] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec3i operator*(int scalar, const MfVec3i &v) { 
        return v * scalar; 
    }

    /// Division by a scalar operator.
    friend MfVec3i operator/(const MfVec3i &v, int scalar) {
	return MfVec3i(int(v._data[0] / scalar), 
                       int(v._data[1] / scalar),
                       int(v._data[2] / scalar));
    }

    /// Unary component-wise addition of two vectors.
    MfVec3i & operator+=(const MfVec3i &v) {
	_data[0] += v._data[0]; 
	_data[1] += v._data[1]; 
	_data[2] += v._data[2]; 
	return *this;
    }

    /// Unary component-wise subtraction of two vectors.
    MfVec3i & operator-=(const MfVec3i &v) {
	_data[0] -= v._data[0]; 
	_data[1] -= v._data[1]; 
	_data[2] -= v._data[2]; 
	return *this;
    }

    /// Unary component-wise multiplication of two vectors.
    MfVec3i & operator*=(int d) {
	_data[0] = int(_data[0] * d); 
	_data[1] = int(_data[1] * d); 
	_data[2] = int(_data[2] * d); 
	return *this;
    }

    /// Unary component-wise division by given scalar.
    MfVec3i & operator/=(int d) {
	_data[0] = int(_data[0] / d); 
	_data[1] = int(_data[1] / d); 
	_data[2] = int(_data[2] / d); 
	return *this;
    }

    /// A negation of this vector.
    MfVec3i operator-() const {
	return MfVec3i(-_data[0], -_data[1], -_data[2]);
    }

    /// Dot product
    int Dot(const MfVec3i &vec) const {
	return 
            _data[0] * vec._data[0] + 
            _data[1] * vec._data[1] +
            _data[2] * vec._data[2];
    }

    /// Returns L2 norm (length) of the vector.
    double GetLength() const {
        return sqrt(double(Dot(*this)));
    }

    /// Output vector to given stream \p os. To be used strictly for
    /// debugging or non-formal presentation of data since there is no
    /// guarantee that the formatting of the data won't change.
    friend std::ostream & operator<<(std::ostream &os, const MfVec3i &vec);
    friend std::istream & operator>>(std::istream &is, MfVec3i &vec);

  private:
    int _data[3];
};

#endif // __MF_VEC3I_H__
