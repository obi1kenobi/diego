#include "JsonSerializer.h"

void
JsonSerializer::Indent()
{
    for (int i = 0; i < _level; ++i) {
        _os << "  ";
    }
}

void
JsonSerializer::Open()
{
    Indent(os);
    os << "{\n";
    ++_level;
}

void
JsonSerializer::Close()
{
    assert(_level > 0);
    --_level;
    Indent();
    os << "}\n";
}

void
JsonSerializer::AddKeyValue(const std::string &key, const std::string &value)
{
}
