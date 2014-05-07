#include "Vec4f.h"

#include "Vec4d.h"
#include "Vec4i.h"

#include <istream>
#include <ostream>

MfVec4f::MfVec4f(const MfVec4d &vdouble) 
{
    _data[0] = float(vdouble[0]);
    _data[1] = float(vdouble[1]);
    _data[2] = float(vdouble[2]);
    _data[3] = float(vdouble[3]);
}

MfVec4f::MfVec4f(const MfVec4i &vint) {
    _data[0] = static_cast<float>(vint[0]);
    _data[1] = static_cast<float>(vint[1]);
    _data[2] = static_cast<float>(vint[2]);
    _data[3] = static_cast<float>(vint[3]);
}

std::ostream &
operator<<(std::ostream &os, const MfVec4f &vec) 
{
    os << vec._data[0] << " " << vec._data[1] << " " << vec._data[2] << " " << vec._data[3];
    return os;
}

std::istream &
operator>>(std::istream &is, MfVec4f &vec) 
{
    is >> vec._data[0];
    is >> vec._data[1];
    is >> vec._data[2];
    is >> vec._data[3];
    return is;
}
