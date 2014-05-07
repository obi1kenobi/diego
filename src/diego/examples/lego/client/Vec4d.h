#ifndef __MF_VEC4D_H__
#define __MF_VEC4D_H__

#include <cstddef>
#include <cmath>
#include <iosfwd>

///
/// Vector of 4 components of type double.
///
class MfVec3d;
class MfVec4f;
class MfVec4i;

class MfVec4d {
  public:
    /// Default constructor makes an undefined vector.
    MfVec4d() {}

    /// A constructor that initializes the vector from a 3D vector and a
    /// 4-th component value.
    MfVec4d(const MfVec3d &vec, double w);

    /// A constructor that initializes the vector from a double array
    explicit MfVec4d(const double varray[4]) { 
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
	_data[3] = varray[3]; 
    }

    /// A constructor that initializes the vector from the passed values.
    MfVec4d(double v0, double v1, double v2, double v3) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
	_data[3] = v3;
    }

    /// A constructor that initializes the vector by smearing the given
    /// value along all vector components.
    explicit MfVec4d(double smearValue) {
        _data[0] = smearValue; 
        _data[1] = smearValue; 
        _data[2] = smearValue; 
        _data[3] = smearValue; 
    }

    /// Set vector values from given array.
    MfVec4d & Set(const double varray[4]) {
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
	_data[3] = varray[3]; 
	return *this;
    }

    /// Sets vector values from given individual element values.
    MfVec4d & Set(double v0, double v1, double v2, double v3) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
	_data[3] = v3;
	return *this;
    }

    /// A constructor that converts a float to double vector.
    MfVec4d(const MfVec4f &vfloat);

    /// A constructor that converts an int to double vector.
    MfVec4d(const MfVec4i &vint);

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
    MfVec4d & operator=(const MfVec4d &v) {
	_data[0] = v[0]; 
	_data[1] = v[1]; 
	_data[2] = v[2]; 
	_data[3] = v[3]; 
	return *this;
    }

    /// Equality operator.
    bool operator==(const MfVec4d &v) const {
	return 
            _data[0] == v._data[0] && 
            _data[1] == v._data[1] && 
            _data[2] == v._data[2] &&
            _data[3] == v._data[3];
    }

    /// Inequality operator.
    bool operator!=(const MfVec4d &v) const {
	return ! (*this == v);
    }

    /// Addition operator.
    friend MfVec4d operator+(const MfVec4d &v1, const MfVec4d &v2) {
	return MfVec4d(v1._data[0] + v2._data[0], 
                       v1._data[1] + v2._data[1],
                       v1._data[2] + v2._data[2],
                       v1._data[3] + v2._data[3]);
    }

    /// Subtraction operator.
    friend MfVec4d operator-(const MfVec4d &v1, const MfVec4d &v2) {
	return MfVec4d(v1._data[0] - v2._data[0], 
                       v1._data[1] - v2._data[1],
                       v1._data[2] - v2._data[2],
                       v1._data[3] - v2._data[3]);
    }

    /// Multiplication by a scalar operator.
    friend MfVec4d operator*(const MfVec4d &v, double scalar) {
	return MfVec4d(double(v._data[0] * scalar), 
                       double(v._data[1] * scalar),
                       double(v._data[2] * scalar),
                       double(v._data[3] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec4d operator*(double scalar, const MfVec4d &v) { 
        return v * scalar; 
    }

    /// Division by a scalar operator.
    friend MfVec4d operator/(const MfVec4d &v, double scalar) {
	return v * (double(1) / scalar);
    }

    /// Unary component-wise addition of two vectors.
    MfVec4d & operator+=(const MfVec4d &v) {
	_data[0] += v._data[0]; 
	_data[1] += v._data[1]; 
	_data[2] += v._data[2]; 
	_data[3] += v._data[3]; 
	return *this;
    }

    /// Unary component-wise subtraction of two vectors.
    MfVec4d & operator-=(const MfVec4d &v) {
	_data[0] -= v._data[0]; 
	_data[1] -= v._data[1]; 
	_data[2] -= v._data[2]; 
	_data[3] -= v._data[3]; 
	return *this;
    }

    /// Unary component-wise multiplication of two vectors.
    MfVec4d & operator*=(double d) {
	_data[0] = double(_data[0] * d); 
	_data[1] = double(_data[1] * d); 
	_data[2] = double(_data[2] * d); 
	_data[3] = double(_data[3] * d); 
	return *this;
    }

    /// Unary component-wise division by given scalar.
    MfVec4d & operator/=(double d) {
	return (*this *= 1.0 / d);
    }

    /// A negation of this vector.
    MfVec4d operator-() const {
	return MfVec4d(-_data[0], -_data[1], -_data[2], -_data[3]);
    }

    /// Dot product
    double Dot(const MfVec4d &vec) const {
	return 
            _data[0] * vec._data[0] + 
            _data[1] * vec._data[1] +
            _data[2] * vec._data[2] +
            _data[3] * vec._data[3];
    }

    /// Returns L2 norm (length) of the vector.
    double GetLength() const {
        return sqrt(Dot(*this));
    }

    /// Returns a unit-length version of this vector.
    MfVec4d GetNormalized() const {
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
    friend std::ostream & operator<<(std::ostream &os, const MfVec4d &vec);
    friend std::istream & operator>>(std::istream &is, MfVec4d &vec);

  private:
    double _data[4];
};

#endif // __MF_VEC4D_H__
