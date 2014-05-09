#include "Vec2i.h"

#include <istream>
#include <ostream>

std::ostream &
operator<<(std::ostream &os, const MfVec2i &vec) 
{
    os << vec._data[0] << " " << vec._data[1];
    return os;
}

std::istream &
operator>>(std::istream &is, MfVec2i &vec) 
{
    is >> vec._data[0];
    is >> vec._data[1];
    return is;
}
