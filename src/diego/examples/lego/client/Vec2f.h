#ifndef __MF_VEC2F_H__
#define __MF_VEC2F_H__

#include <cstddef>
#include <cmath>
#include <iosfwd>

///
/// Vector of 2 components of type float.
///
class MfVec2d;
class MfVec2i;

class MfVec2f {
  public:
    /// Default constructor makes an undefined vector.
    MfVec2f() {}

    /// A constructor that initializes the vector from a float array
    explicit MfVec2f(const float varray[2]) { 
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
    }

    /// A constructor that initializes the vector from the passed values.
    MfVec2f(float v0, float v1) {
	_data[0] = v0; 
	_data[1] = v1; 
    }

    /// A constructor that initializes the vector by smearing the given
    /// value along all vector components.
    explicit MfVec2f(float smearValue) {
        _data[0] = smearValue; 
        _data[1] = smearValue; 
    }

    /// Set vector values from given array.
    MfVec2f & Set(const float varray[2]) {
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	return *this;
    }

    /// Sets vector values from given individual element values.
    MfVec2f & Set(float v0, float v1) {
	_data[0] = v0; 
	_data[1] = v1; 
	return *this;
    }

    /// A constructor that converts a double to float vector.
    MfVec2f(const MfVec2d &vdouble);

    /// A constructor that converts an int to float vector.
    MfVec2f(const MfVec2i &vint);

    /// Returns vector as an array of float values.
    const float * Get() const { 
        return _data; 
    }

    /// Allow indexed access to underlying values.
    float & operator[](size_t i) {
        return _data[i]; 
    }

    /// Allow indexed access to underlying const values.
    const float & operator[](size_t i) const { 
        return _data[i]; 
    }

    /// Assignment operator.
    MfVec2f & operator=(const MfVec2f &v) {
	_data[0] = v[0]; 
	_data[1] = v[1]; 
	return *this;
    }

    /// Equality operator.
    bool operator==(const MfVec2f &v) const {
	return _data[0] == v._data[0] && _data[1] == v._data[1];
    }

    /// Inequality operator.
    bool operator!=(const MfVec2f &v) const {
	return ! (*this == v);
    }

    /// Addition operator.
    friend MfVec2f operator+(const MfVec2f &v1, const MfVec2f &v2) {
	return MfVec2f(v1._data[0] + v2._data[0], v1._data[1] + v2._data[1]);
    }

    /// Subtraction operator.
    friend MfVec2f operator-(const MfVec2f &v1, const MfVec2f &v2) {
	return MfVec2f(v1._data[0] - v2._data[0], v1._data[1] - v2._data[1]);
    }

    /// Multiplication by a scalar operator.
    friend MfVec2f operator*(const MfVec2f &v, float scalar) {
	return MfVec2f(float(v._data[0] * scalar), float(v._data[1] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec2f operator*(float scalar, const MfVec2f &v) { 
        return v * scalar; 
    }

    /// Division by a scalar operator.
    friend MfVec2f operator/(const MfVec2f &v, float s) {
	return v * (float(1) / s);
    }

    /// Unary component-wise addition of two vectors.
    MfVec2f & operator+=(const MfVec2f &v) {
	_data[0] += v._data[0]; 
	_data[1] += v._data[1]; 
	return *this;
    }

    /// Unary component-wise subtraction of two vectors.
    MfVec2f & operator-=(const MfVec2f &v) {
	_data[0] -= v._data[0]; 
	_data[1] -= v._data[1]; 
	return *this;
    }

    /// Unary component-wise multiplication of two vectors.
    MfVec2f & operator*=(float d) {
	_data[0] = float(_data[0] * d); 
	_data[1] = float(_data[1] * d); 
	return *this;
    }

    /// Unary component-wise division by given scalar.
    MfVec2f & operator/=(float d) {
        return (*this *= 1.0f / d);
    }

    /// A negation of this vector.
    MfVec2f operator-() const {
	return MfVec2f(-_data[0], -_data[1]);
    }

    /// Dot product
    float Dot(const MfVec2f &vec) const {
	return _data[0] * vec._data[0] + _data[1] * vec._data[1];
    }

    /// Returns L2 norm (length) of the vector.
    float GetLength() const {
        return sqrt(Dot(*this));
    }

    /// Returns a unit-length version of this vector.
    MfVec2f GetNormalized() const {
	float length = GetLength();
	return *this / length;
    }

    /// Normalizes vector to unit length.
    float Normalize() {
	float length = GetLength();
        *this = *this / length;
	return length;
    }

    /// Output vector to given stream \p os. To be used strictly for
    /// debugging or non-formal presentation of data since there is no
    /// guarantee that the formatting of the data won't change.
    friend std::ostream & operator<<(std::ostream &os, const MfVec2f &vec);

  private:
    float _data[2];
};

#endif // __MF_VEC2F_H__
