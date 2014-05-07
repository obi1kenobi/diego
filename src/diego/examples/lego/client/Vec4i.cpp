#include "Vec4i.h"

#include "Vec4f.h"
#include "Vec4d.h"

#include <ostream>

MfVec4i::MfVec4i(const MfVec4f &vfloat) 
{
    _data[0] = int32_t(vfloat[0]);
    _data[1] = int32_t(vfloat[1]);
    _data[2] = int32_t(vfloat[2]);
    _data[3] = int32_t(vfloat[3]);
}

MfVec4i::MfVec4i(const MfVec4d &vdouble) 
{
    _data[0] = int32_t(vdouble[0]);
    _data[1] = int32_t(vdouble[1]);
    _data[2] = int32_t(vdouble[2]);
    _data[3] = int32_t(vdouble[3]);
}

std::ostream &
operator<<(std::ostream &os, const MfVec4i &vec) 
{
    os 
        << "(" 
        << vec._data[0] << ", " << vec._data[1] << ", " 
        << vec._data[2] << ", " << vec._data[3] << ")";
    return os;
}
