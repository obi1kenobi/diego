#ifndef __MF_VEC2D_H__
#define __MF_VEC2D_H__

#include <cstddef>
#include <cmath>
#include <iosfwd>

///
/// Vector of 2 components of type double.
///
class MfVec2f;
class MfVec2i;

class MfVec2d {
  public:
    /// Default constructor makes an undefined vector.
    MfVec2d() {}

    /// A constructor that initializes the vector from a double array
    explicit MfVec2d(const double varray[2]) { 
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
    }

    /// A constructor that initializes the vector from the passed values.
    MfVec2d(double v0, double v1) {
	_data[0] = v0; 
	_data[1] = v1; 
    }

    /// A constructor that initializes the vector by smearing the given
    /// value along all vector components.
    explicit MfVec2d(double smearValue) {
        _data[0] = smearValue; 
        _data[1] = smearValue; 
    }

    /// A constructor that converts a float to double vector.
    MfVec2d(const MfVec2f &vfloat);

    /// A constructor that converts an int to float vector.
    MfVec2d(const MfVec2i &vint);

    /// Set vector values from given array.
    MfVec2d & Set(const double varray[2]) {
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	return *this;
    }

    /// Sets vector values from given individual element values.
    MfVec2d & Set(double v0, double v1) {
	_data[0] = v0; 
	_data[1] = v1; 
	return *this;
    }

    /// Returns vector as an array of double values.
    const double * Get() const { 
        return _data; 
    }

    /// Allow indexed access to underlying values.
    double & operator[](size_t i) {
        return _data[i]; 
    }

    /// Allow indexed access to underlying const values.
    const double & operator[](size_t i) const { 
        return _data[i]; 
    }

    /// Assignment operator.
    MfVec2d & operator=(const MfVec2d &v) {
	_data[0] = v[0]; 
	_data[1] = v[1]; 
	return *this;
    }

    /// Equality operator.
    bool operator==(const MfVec2d &v) const {
	return _data[0] == v._data[0] && _data[1] == v._data[1];
    }

    /// Inequality operator.
    bool operator!=(const MfVec2d &v) const {
	return ! (*this == v);
    }

    /// Addition operator.
    friend MfVec2d operator+(const MfVec2d &v1, const MfVec2d &v2) {
	return MfVec2d(v1._data[0] + v2._data[0], v1._data[1] + v2._data[1]);
    }

    /// Subtraction operator.
    friend MfVec2d operator-(const MfVec2d &v1, const MfVec2d &v2) {
	return MfVec2d(v1._data[0] - v2._data[0], v1._data[1] - v2._data[1]);
    }

    /// Multiplication by a scalar operator.
    friend MfVec2d operator*(const MfVec2d &v, double scalar) {
	return MfVec2d(double(v._data[0] * scalar), double(v._data[1] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec2d operator*(double scalar, const MfVec2d &v) { 
        return v * scalar; 
    }

    /// Division by a scalar operator.
    friend MfVec2d operator/(const MfVec2d &v, double s) {
	return v * (double(1) / s);
    }

    /// Unary component-wise addition of two vectors.
    MfVec2d & operator+=(const MfVec2d &v) {
	_data[0] += v._data[0]; 
	_data[1] += v._data[1]; 
	return *this;
    }

    /// Unary component-wise subtraction of two vectors.
    MfVec2d & operator-=(const MfVec2d &v) {
	_data[0] -= v._data[0]; 
	_data[1] -= v._data[1]; 
	return *this;
    }

    /// Unary component-wise multiplication of two vectors.
    MfVec2d & operator*=(double d) {
	_data[0] = double(_data[0] * d); 
	_data[1] = double(_data[1] * d); 
	return *this;
    }

    /// Unary component-wise division by given scalar.
    MfVec2d & operator/=(double d) {
	return (*this *= 1. / d);
    }

    /// A negation of this vector.
    MfVec2d operator-() const {
	return MfVec2d(-_data[0], -_data[1]);
    }

    /// Dot product
    double Dot(const MfVec2d &vec) const {
	return _data[0] * vec._data[0] + _data[1] * vec._data[1];
    }

    /// Returns L2 norm (length) of the vector.
    double GetLength() const {
        return sqrt(Dot(*this));
    }

    /// Returns a unit-length version of this vector.
    MfVec2d GetNormalized() const {
	double length = GetLength();
	return *this / length;
    }

    /// Normalizes vector to unit length.
    double Normalize() {
	double length = GetLength();
        *this = *this / length;
	return length;
    }

    /// Output vector to given stream \p os. To be used strictly for
    /// debugging or non-formal presentation of data since there is no
    /// guarantee that the formatting of the data won't change.
    friend std::ostream & operator<<(std::ostream &os, const MfVec2d &vec);
    friend std::istream & operator>>(std::istream &is, MfVec2d &vec);

  private:
    double _data[2];
};

#endif // __MF_VEC2D_H__
