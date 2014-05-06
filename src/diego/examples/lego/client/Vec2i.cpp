#include "Vec2i.h"

#include <ostream>

std::ostream &
operator<<(std::ostream &os, const MfVec2i &vec) 
{
    os << "(" << vec._data[0] << ", " << vec._data[1] << ")";
    return os;
}
