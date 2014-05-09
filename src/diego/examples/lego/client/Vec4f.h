#ifndef __MF_VEC4F_H__
#define __MF_VEC4F_H__

#include <cstddef>
#include <cmath>
#include <iosfwd>

///
/// Vector of 4 components of type float.
///
class MfVec4d;
class MfVec4i;

class MfVec4f {
  public:
    /// Default constructor makes an undefined vector.
    MfVec4f() {}

    /// A constructor that initializes the vector from a float array
    explicit MfVec4f(const float varray[4]) { 
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
	_data[3] = varray[3]; 
    }

    /// A constructor that initializes the vector from the passed values.
    MfVec4f(float v0, float v1, float v2, float v3) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
	_data[3] = v3;
    }

    /// A constructor that initializes the vector by smearing the given
    /// value along all vector components.
    explicit MfVec4f(float smearValue) {
        _data[0] = smearValue; 
        _data[1] = smearValue; 
        _data[2] = smearValue; 
        _data[3] = smearValue; 
    }

    /// Set vector values from given array.
    MfVec4f & Set(const float varray[4]) {
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
	_data[3] = varray[3]; 
	return *this;
    }

    /// Sets vector values from given individual element values.
    MfVec4f & Set(float v0, float v1, float v2, float v3) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
	_data[3] = v3;
	return *this;
    }

    /// A constructor that converts a double to float vector.
    MfVec4f(const MfVec4d &vdouble);

    /// A constructor that converts an int to float vector.
    MfVec4f(const MfVec4i &vint);

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
    MfVec4f & operator=(const MfVec4f &v) {
	_data[0] = v[0]; 
	_data[1] = v[1]; 
	_data[2] = v[2]; 
	_data[3] = v[3]; 
	return *this;
    }

    /// Equality operator.
    bool operator==(const MfVec4f &v) const {
	return 
            _data[0] == v._data[0] && 
            _data[1] == v._data[1] && 
            _data[2] == v._data[2] &&
            _data[3] == v._data[3];
    }

    /// Inequality operator.
    bool operator!=(const MfVec4f &v) const {
	return ! (*this == v);
    }

    /// Addition operator.
    friend MfVec4f operator+(const MfVec4f &v1, const MfVec4f &v2) {
	return MfVec4f(v1._data[0] + v2._data[0], 
                       v1._data[1] + v2._data[1],
                       v1._data[2] + v2._data[2],
                       v1._data[3] + v2._data[3]);
    }

    /// Subtraction operator.
    friend MfVec4f operator-(const MfVec4f &v1, const MfVec4f &v2) {
	return MfVec4f(v1._data[0] - v2._data[0], 
                       v1._data[1] - v2._data[1],
                       v1._data[2] - v2._data[2],
                       v1._data[3] - v2._data[3]);
    }

    /// Multiplication by a scalar operator.
    friend MfVec4f operator*(const MfVec4f &v, float scalar) {
	return MfVec4f(float(v._data[0] * scalar), 
                       float(v._data[1] * scalar),
                       float(v._data[2] * scalar),
                       float(v._data[3] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec4f operator*(float scalar, const MfVec4f &v) { 
        return v * scalar; 
    }

    /// Division by a scalar operator.
    friend MfVec4f operator/(const MfVec4f &v, float scalar) {
	return v * (float(1) / scalar);
    }

    /// Unary component-wise addition of two vectors.
    MfVec4f & operator+=(const MfVec4f &v) {
	_data[0] += v._data[0]; 
	_data[1] += v._data[1]; 
	_data[2] += v._data[2]; 
	_data[3] += v._data[3]; 
	return *this;
    }

    /// Unary component-wise subtraction of two vectors.
    MfVec4f & operator-=(const MfVec4f &v) {
	_data[0] -= v._data[0]; 
	_data[1] -= v._data[1]; 
	_data[2] -= v._data[2]; 
	_data[3] -= v._data[3]; 
	return *this;
    }

    /// Unary component-wise multiplication of two vectors.
    MfVec4f & operator*=(float d) {
	_data[0] = float(_data[0] * d); 
	_data[1] = float(_data[1] * d); 
	_data[2] = float(_data[2] * d); 
	_data[3] = float(_data[3] * d); 
	return *this;
    }

    /// Unary component-wise division by given scalar.
    MfVec4f & operator/=(float d) {
	return (*this *= 1.0f / d);
    }

    /// A negation of this vector.
    MfVec4f operator-() const {
	return MfVec4f(-_data[0], -_data[1], -_data[2], -_data[3]);
    }

    /// Dot product
    float Dot(const MfVec4f &vec) const {
	return 
            _data[0] * vec._data[0] + 
            _data[1] * vec._data[1] +
            _data[2] * vec._data[2] +
            _data[3] * vec._data[3];
    }

    /// Returns L2 norm (length) of the vector.
    float GetLength() const {
        return sqrt(Dot(*this));
    }

    /// Returns a unit-length version of this vector.
    MfVec4f GetNormalized() const {
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
    friend std::ostream & operator<<(std::ostream &os, const MfVec4f &vec);
    friend std::istream & operator>>(std::istream &is, MfVec4f &vec);

  private:
    float _data[4];
};

#endif // __MF_VEC4F_H__
