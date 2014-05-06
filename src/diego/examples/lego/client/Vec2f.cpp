#include "Vec2f.h"

#include "Vec2d.h"
#include "Vec2i.h"

#include <ostream>

MfVec2f::MfVec2f(const MfVec2d &vdouble) 
{
    _data[0] = float(vdouble[0]);
    _data[1] = float(vdouble[1]);
}

MfVec2f::MfVec2f(const MfVec2i &vint) 
{
    _data[0] = float(vint[0]);
    _data[1] = float(vint[1]);
}

std::ostream &
operator<<(std::ostream &os, const MfVec2f &vec) 
{
    os << "(" << vec._data[0] << ", " << vec._data[1] << ")";
    return os;
}
