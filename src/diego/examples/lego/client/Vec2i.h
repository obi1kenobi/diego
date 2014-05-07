#ifndef __MF_VEC2I_H__
#define __MF_VEC2I_H__

#include "IntTypes.h"

#include <cstddef>
#include <cmath>
#include <iosfwd>

///
/// Vector of 2 components of type int.
///
class MfVec2i {
  public:
    /// Default constructor makes an undefined vector.
    MfVec2i() {}

    /// A constructor that initializes the vector from an int array
    explicit MfVec2i(const int varray[2]) { 
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
    }

    /// A constructor that initializes the vector from the passed values.
    MfVec2i(int v0, int v1) {
	_data[0] = v0; 
	_data[1] = v1; 
    }

    /// A constructor that initializes the vector by smearing the given
    /// value along all vector components.
    explicit MfVec2i(int smearValue) {
        _data[0] = smearValue; 
        _data[1] = smearValue; 
    }

    /// Set vector values from given array.
    MfVec2i & Set(const int varray[2]) {
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	return *this;
    }

    /// Sets vector values from given individual element values.
    MfVec2i & Set(int v0, int v1) {
	_data[0] = v0; 
	_data[1] = v1; 
	return *this;
    }

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
    MfVec2i & operator=(const MfVec2i &v) {
	_data[0] = v[0]; 
	_data[1] = v[1]; 
	return *this;
    }

    /// Equality operator.
    bool operator==(const MfVec2i &v) const {
	return _data[0] == v._data[0] && _data[1] == v._data[1];
    }

    /// Inequality operator.
    bool operator!=(const MfVec2i &v) const {
	return ! (*this == v);
    }

    /// Addition operator.
    friend MfVec2i operator+(const MfVec2i &v1, const MfVec2i &v2) {
	return MfVec2i(v1._data[0] + v2._data[0], v1._data[1] + v2._data[1]);
    }

    /// Subtraction operator.
    friend MfVec2i operator-(const MfVec2i &v1, const MfVec2i &v2) {
	return MfVec2i(v1._data[0] - v2._data[0], v1._data[1] - v2._data[1]);
    }

    /// Multiplication by a scalar operator.
    friend MfVec2i operator*(const MfVec2i &v, int scalar) {
	return MfVec2i(int(v._data[0] * scalar), int(v._data[1] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec2i operator*(int scalar, const MfVec2i &v) { 
        return v * scalar; 
    }

    /// Division by a scalar operator.
    friend MfVec2i operator/(const MfVec2i &v, int scalar) {
	return v * (int(1) / scalar);
    }

    /// Multiplication by a scalar operator.
    friend MfVec2i operator*(const MfVec2i &v, double scalar) {
	return MfVec2i(int(v._data[0] * scalar), int(v._data[1] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec2i operator*(double scalar, const MfVec2i &v) { 
        return v * scalar; 
    }

    /// Division by a scalar operator.
    friend MfVec2i operator/(const MfVec2i &v, double scalar) {
	return v * (int(1) / scalar);
    }

    /// Unary component-wise addition of two vectors.
    MfVec2i & operator+=(const MfVec2i &v) {
	_data[0] += v._data[0]; 
	_data[1] += v._data[1]; 
	return *this;
    }

    /// Unary component-wise subtraction of two vectors.
    MfVec2i & operator-=(const MfVec2i &v) {
	_data[0] -= v._data[0]; 
	_data[1] -= v._data[1]; 
	return *this;
    }

    /// Unary component-wise multiplication of two vectors.
    MfVec2i & operator*=(int d) {
	_data[0] = int(_data[0] * d); 
	_data[1] = int(_data[1] * d); 
	return *this;
    }

    /// Unary component-wise division by given scalar.
    MfVec2i & operator/=(int d) {
	_data[0] = int(_data[0] / d); 
	_data[1] = int(_data[1] / d); 
	return *this;
    }

    /// A negation of this vector.
    MfVec2i operator-() const {
	return MfVec2i(-_data[0], -_data[1]);
    }

    /// Dot product
    int Dot(const MfVec2i &vec) const {
	return _data[0] * vec._data[0] + _data[1] * vec._data[1];
    }

    /// Returns L2 norm (length) of the vector.
    double GetLength() const {
        return sqrt(double(Dot(*this)));
    }

    /// Output vector to given stream \p os. To be used strictly for
    /// debugging or non-formal presentation of data since there is no
    /// guarantee that the formatting of the data won't change.
    friend std::ostream & operator<<(std::ostream &os, const MfVec2i &vec);

  private:
    int _data[2];
};

#endif // __MF_VEC2I_H__
