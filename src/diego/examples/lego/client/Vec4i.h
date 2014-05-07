#ifndef __MF_VEC4I_H__
#define __MF_VEC4I_H__

#include "IntTypes.h"

#include <cstddef>
#include <cmath>
#include <iosfwd>

///
/// Vector of 4 components of type int.
///
class MfVec4f;
class MfVec4d;

class MfVec4i {
  public:
    /// Default constructor makes an undefined vector.
    MfVec4i() {}

    /// A constructor that initializes the vector from a int array
    explicit MfVec4i(const int varray[4]) { 
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
	_data[3] = varray[3]; 
    }

    /// A constructor that initializes the vector from the passed values.
    MfVec4i(int v0, int v1, int v2, int v3) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
	_data[3] = v3;
    }

    /// A constructor that initializes the vector by smearing the given
    /// value along all vector components.
    explicit MfVec4i(int smearValue) {
        _data[0] = smearValue; 
        _data[1] = smearValue; 
        _data[2] = smearValue; 
        _data[3] = smearValue; 
    }

    /// Set vector values from given array.
    MfVec4i & Set(const int varray[4]) {
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
	_data[3] = varray[3]; 
	return *this;
    }

    /// Sets vector values from given individual element values.
    MfVec4i & Set(int v0, int v1, int v2, int v3) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
	_data[3] = v3;
	return *this;
    }

    /// A constructor that converts a float to int vector.
    MfVec4i(const MfVec4f &vfloat);

    /// A constructor that converts a double to int vector.
    MfVec4i(const MfVec4d &vint);

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
    MfVec4i & operator=(const MfVec4i &v) {
	_data[0] = v[0]; 
	_data[1] = v[1]; 
	_data[2] = v[2]; 
	_data[3] = v[3]; 
	return *this;
    }

    /// Equality operator.
    bool operator==(const MfVec4i &v) const {
	return 
            _data[0] == v._data[0] && 
            _data[1] == v._data[1] && 
            _data[2] == v._data[2] &&
            _data[3] == v._data[3];
    }

    /// Inequality operator.
    bool operator!=(const MfVec4i &v) const {
	return ! (*this == v);
    }

    /// Addition operator.
    friend MfVec4i operator+(const MfVec4i &v1, const MfVec4i &v2) {
	return MfVec4i(v1._data[0] + v2._data[0], 
                       v1._data[1] + v2._data[1],
                       v1._data[2] + v2._data[2],
                       v1._data[3] + v2._data[3]);
    }

    /// Subtraction operator.
    friend MfVec4i operator-(const MfVec4i &v1, const MfVec4i &v2) {
	return MfVec4i(v1._data[0] - v2._data[0], 
                       v1._data[1] - v2._data[1],
                       v1._data[2] - v2._data[2],
                       v1._data[3] - v2._data[3]);
    }

    /// Multiplication by a scalar operator.
    friend MfVec4i operator*(const MfVec4i &v, int scalar) {
	return MfVec4i(int(v._data[0] * scalar), 
                       int(v._data[1] * scalar),
                       int(v._data[2] * scalar),
                       int(v._data[3] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec4i operator*(const MfVec4i &v, double scalar) {
	return MfVec4i(int(v._data[0] * scalar), 
                       int(v._data[1] * scalar),
                       int(v._data[2] * scalar),
                       int(v._data[3] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec4i operator*(int scalar, const MfVec4i &v) { 
        return v * scalar; 
    }

    /// Division by a scalar operator.
    friend MfVec4i operator/(const MfVec4i &v, int scalar) {
	return v * (1.0 / scalar);
    }

    /// Unary component-wise addition of two vectors.
    MfVec4i & operator+=(const MfVec4i &v) {
	_data[0] += v._data[0]; 
	_data[1] += v._data[1]; 
	_data[2] += v._data[2]; 
	_data[3] += v._data[3]; 
	return *this;
    }

    /// Unary component-wise subtraction of two vectors.
    MfVec4i & operator-=(const MfVec4i &v) {
	_data[0] -= v._data[0]; 
	_data[1] -= v._data[1]; 
	_data[2] -= v._data[2]; 
	_data[3] -= v._data[3]; 
	return *this;
    }

    /// Unary component-wise multiplication of two vectors.
    MfVec4i & operator*=(int d) {
	_data[0] = int(_data[0] * d); 
	_data[1] = int(_data[1] * d); 
	_data[2] = int(_data[2] * d); 
	_data[3] = int(_data[3] * d); 
	return *this;
    }

    /// Unary component-wise division by given scalar.
    MfVec4i & operator/=(int scalar) {
	    _data[0] = _data[0] / scalar;
        _data[1] = _data[1] / scalar;
        _data[2] = _data[2] / scalar;
        _data[3] = _data[3] / scalar;
        return *this;
    }

    /// A negation of this vector.
    MfVec4i operator-() const {
	return MfVec4i(-_data[0], -_data[1], -_data[2], -_data[3]);
    }

    /// Dot product
    int Dot(const MfVec4i &vec) const {
	return 
            _data[0] * vec._data[0] + 
            _data[1] * vec._data[1] +
            _data[2] * vec._data[2] +
            _data[3] * vec._data[3];
    }

    /// Returns L2 norm (length) of the vector.
    double GetLength() const {
        return sqrt(double(Dot(*this)));
    }

    /// Output vector to given stream \p os. To be used strictly for
    /// debugging or non-formal presentation of data since there is no
    /// guarantee that the formatting of the data won't change.
    friend std::ostream & operator<<(std::ostream &os, const MfVec4i &vec);
    friend std::istream & operator>>(std::istream &is, MfVec4i &vec);

  private:
    int _data[4];
};

#endif // __MF_VEC4I_H__
