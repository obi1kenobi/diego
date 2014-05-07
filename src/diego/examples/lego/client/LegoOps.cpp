#include "LegoOps.h"

#include <assert.h>

#include "JsonSerializer.h"

void
LegoOp::Serialize(std::ostringstream &os)
{
    switch (_type) {
    case CREATE_BRICK: os << "CreateBrick"; break;
    case MODIFY_POSITION: os << "ModifyPosition"; break;
    case DELETE_BRICK: os << "DeleteBrick"; break;
    }
}

void
LegoOpCreateBrick::Serialize(std::ostringstream &os)
{
    LegoOp::Serialize(os);

    os << " " << _position << " " << _size << " " << _orientation << " " << _color << "\n";
}

void
LegoOpModifyPosition::Serialize(std::ostringstream &os)
{
}
