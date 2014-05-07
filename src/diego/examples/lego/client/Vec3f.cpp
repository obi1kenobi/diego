#include "Vec3f.h"

#include "Vec3d.h"
#include "Vec3i.h"

#include <ostream>

MfVec3f::MfVec3f(const MfVec3d &vdouble) 
{
    _data[0] = float(vdouble[0]);
    _data[1] = float(vdouble[1]);
    _data[2] = float(vdouble[2]);
}

MfVec3f::MfVec3f(const MfVec3i &vint) {
    _data[0] = float(vint[0]);
    _data[1] = float(vint[1]);
    _data[2] = float(vint[2]);
}

std::ostream &
operator<<(std::ostream &os, const MfVec3f &vec) 
{
    os << "(" << vec._data[0] << ", " << vec._data[1] << ", " << vec._data[2] << ")";
    return os;
}
