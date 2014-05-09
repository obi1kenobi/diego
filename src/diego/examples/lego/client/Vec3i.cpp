#include "Vec3f.h"

#include "Vec3d.h"
#include "Vec3i.h"

#include <istream>
#include <ostream>

MfVec3i::MfVec3i(const MfVec3d &vdouble) 
{
    _data[0] = int32_t(vdouble[0]);
    _data[1] = int32_t(vdouble[1]);
    _data[2] = int32_t(vdouble[2]);
}

MfVec3i::MfVec3i(const MfVec3f &vfloat) {
    _data[0] = int32_t(vfloat[0]);
    _data[1] = int32_t(vfloat[1]);
    _data[2] = int32_t(vfloat[2]);
}

std::ostream &
operator<<(std::ostream &os, const MfVec3i &vec) 
{
    os << vec._data[0] << " " << vec._data[1] << " " << vec._data[2];
    return os;
}

std::istream &
operator>>(std::istream &is, MfVec3i &vec) 
{
    is >> vec._data[0];
    is >> vec._data[1];
    is >> vec._data[2];
    return is;
}
