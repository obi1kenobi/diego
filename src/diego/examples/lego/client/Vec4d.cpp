#include "Vec4d.h"

#include "Vec3d.h"
#include "Vec4f.h"
#include "Vec4i.h"

#include <ostream>

MfVec4d::MfVec4d(const MfVec3d &vec, double w) 
{
    _data[0] = vec[0]; 
    _data[1] = vec[1]; 
    _data[2] = vec[2]; 
    _data[3] = w;
}

MfVec4d::MfVec4d(const MfVec4f &vfloat) 
{
    _data[0] = vfloat[0];
    _data[1] = vfloat[1];
    _data[2] = vfloat[2];
    _data[3] = vfloat[3];
}

MfVec4d::MfVec4d(const MfVec4i &vint) {
    _data[0] = vint[0];
    _data[1] = vint[1];
    _data[2] = vint[2];
    _data[3] = vint[3];
}

std::ostream &
operator<<(std::ostream &os, const MfVec4d &vec) 
{
    os 
        << "(" 
        << vec._data[0] << ", " << vec._data[1] << ", " 
        << vec._data[2] << ", " << vec._data[3] << ")";
    return os;
}
