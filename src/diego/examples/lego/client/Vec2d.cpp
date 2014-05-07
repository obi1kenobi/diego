#include "Vec2d.h"

#include "Vec2f.h"
#include "Vec2i.h"

#include <istream>
#include <ostream>

MfVec2d::MfVec2d(const MfVec2f &vfloat) 
{
    _data[0] = vfloat[0];
    _data[1] = vfloat[1];
}

MfVec2d::MfVec2d(const MfVec2i &vint) 
{
    _data[0] = vint[0];
    _data[1] = vint[1];
}

std::ostream &
operator<<(std::ostream &os, const MfVec2d &vec) 
{
    os << vec._data[0] << " " << vec._data[1];
    return os;
}

std::istream &
operator>>(std::istream &is, MfVec2d &vec) 
{
    is >> vec._data[0];
    is >> vec._data[1];
    return is;
}
