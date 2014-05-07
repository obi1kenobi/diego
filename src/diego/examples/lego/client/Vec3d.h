#ifndef __MF_VEC3D_H__
#define __MF_VEC3D_H__

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <iosfwd>

///
/// Vector of 3 components of type double.
///
class MfVec3f;
class MfVec3i;

class MfVec3d {
  public:
    /// Default constructor makes an undefined vector.
    MfVec3d() {}

    /// A constructor that initializes the vector from a double array
    explicit MfVec3d(const double varray[3]) { 
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
    }

    /// A constructor that initializes the vector from the passed values.
    MfVec3d(double v0, double v1, double v2) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
    }

    /// A constructor that initializes the vector by smearing the given
    /// value along all vector components.
    explicit MfVec3d(double smearValue) {
        _data[0] = smearValue; 
        _data[1] = smearValue; 
        _data[2] = smearValue; 
    }

    /// Set vector values from given array.
    MfVec3d & Set(const double varray[3]) {
	_data[0] = varray[0]; 
	_data[1] = varray[1]; 
	_data[2] = varray[2]; 
	return *this;
    }

    /// Sets vector values from given individual element values.
    MfVec3d & Set(double v0, double v1, double v2) {
	_data[0] = v0;
	_data[1] = v1;
	_data[2] = v2;
	return *this;
    }

    /// A constructor that converts a float to double vector.
    MfVec3d(const MfVec3f &vdouble);

    /// A constructor that converts an int to double vector.
    MfVec3d(const MfVec3i &vint);

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
    MfVec3d & operator=(const MfVec3d &v) {
	_data[0] = v[0]; 
	_data[1] = v[1]; 
	_data[2] = v[2]; 
	return *this;
    }

    /// Equality operator.
    bool operator==(const MfVec3d &v) const {
	return 
            _data[0] == v._data[0] && 
            _data[1] == v._data[1] && 
            _data[2] == v._data[2];;
    }

    /// Inequality operator.
    bool operator!=(const MfVec3d &v) const {
	return ! (*this == v);
    }

    /// Addition operator.
    friend MfVec3d operator+(const MfVec3d &v1, const MfVec3d &v2) {
	return MfVec3d(v1._data[0] + v2._data[0], 
                       v1._data[1] + v2._data[1],
                       v1._data[2] + v2._data[2]);
    }

    /// Subtraction operator.
    friend MfVec3d operator-(const MfVec3d &v1, const MfVec3d &v2) {
	return MfVec3d(v1._data[0] - v2._data[0], 
                       v1._data[1] - v2._data[1],
                       v1._data[2] - v2._data[2]);
    }

    /// Multiplication by a scalar operator.
    friend MfVec3d operator*(const MfVec3d &v, double scalar) {
	return MfVec3d(double(v._data[0] * scalar), 
                       double(v._data[1] * scalar),
                       double(v._data[2] * scalar));
    }

    /// Multiplication by a scalar operator.
    friend MfVec3d operator*(double scalar, const MfVec3d &v) { 
        return v * scalar; 
    }

    /// Division by a scalar operator.
    friend MfVec3d operator/(const MfVec3d &v, double s) {
	return v * (double(1) / s);
    }

    /// Unary component-wise addition of two vectors.
    MfVec3d & operator+=(const MfVec3d &v) {
	_data[0] += v._data[0]; 
	_data[1] += v._data[1]; 
	_data[2] += v._data[2]; 
	return *this;
    }

    /// Unary component-wise subtraction of two vectors.
    MfVec3d & operator-=(const MfVec3d &v) {
	_data[0] -= v._data[0]; 
	_data[1] -= v._data[1]; 
	_data[2] -= v._data[2]; 
	return *this;
    }

    /// Unary component-wise multiplication of two vectors.
    MfVec3d & operator*=(double d) {
	_data[0] = double(_data[0] * d); 
	_data[1] = double(_data[1] * d); 
	_data[2] = double(_data[2] * d); 
	return *this;
    }

    /// Unary component-wise division by given scalar.
    MfVec3d & operator/=(double d) {
	return (*this *= 1.0 / d);
    }

    /// A negation of this vector.
    MfVec3d operator-() const {
	return MfVec3d(-_data[0], -_data[1], -_data[2]);
    }

    /// Dot product
    double Dot(const MfVec3d &vec) const {
	return 
            _data[0] * vec._data[0] + 
            _data[1] * vec._data[1] +
            _data[2] * vec._data[2];
    }

    /// Cross product
    MfVec3d Cross(const MfVec3d &v) const {
	return 
            MfVec3d(_data[1] * v._data[2] - _data[2] * v._data[1],
                    _data[2] * v._data[0] - _data[0] * v._data[2],
                    _data[0] * v._data[1] - _data[1] * v._data[0]);
    }

    /// Returns L2 norm (length) of the vector.
    double GetLength() const {
        return sqrt(Dot(*this));
    }

    /// Returns a unit-length version of this vector.
    MfVec3d GetNormalized() const {
	double length = GetLength();
	return *this / length;
    }

    /// Normalizes vector to unit length.
    double Normalize() {
	double length = GetLength();
        *this = *this / length;
	return length;
    }

    /// Returns the minimum vector component-wise as compared to given vector.
    MfVec3d GetMin(const MfVec3d &vec) const {
        return MfVec3d(
            std::min(_data[0], vec._data[0]),
            std::min(_data[1], vec._data[1]),
            std::min(_data[2], vec._data[2]));
    }

    /// Returns the maximum vector component-wise as compared to given vector.
    MfVec3d GetMax(const MfVec3d &vec) const {
        return MfVec3d(
            std::max(_data[0], vec._data[0]),
            std::max(_data[1], vec._data[1]),
            std::max(_data[2], vec._data[2]));
    }

    double GetMinComponent() const {
        return std::min(std::min(_data[0], _data[1]), _data[2]);
    }

    double GetMaxComponent() const {
        return std::max(std::max(_data[0], _data[1]), _data[2]);
    }

    /// Output vector to given stream \p os. To be used strictly for
    /// debugging or non-formal presentation of data since there is no
    /// guarantee that the formatting of the data won't change.
    friend std::ostream & operator<<(std::ostream &os, const MfVec3d &vec);

  private:
    double _data[3];
};

#endif // __MF_VEC3D_H__
