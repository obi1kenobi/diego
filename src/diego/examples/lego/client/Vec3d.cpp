#include "Vec3d.h"

#include "Vec3f.h"
#include "Vec3i.h"

#include <istream>
#include <ostream>

MfVec3d::MfVec3d(const MfVec3f &vfloat) 
{
    _data[0] = vfloat[0];
    _data[1] = vfloat[1];
    _data[2] = vfloat[2];
}

MfVec3d::MfVec3d(const MfVec3i &vint) {
    _data[0] = vint[0];
    _data[1] = vint[1];
    _data[2] = vint[2];
}

std::ostream &
operator<<(std::ostream &os, const MfVec3d &vec) 
{
    os << vec._data[0] << " " << vec._data[1] << " " << vec._data[2];
    return os;
}

std::istream &
operator>>(std::istream &is, MfVec3d &vec) 
{
    is >> vec._data[0];
    is >> vec._data[1];
    is >> vec._data[2];
    return is;
}
