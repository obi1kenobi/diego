#include "LegoOps.h"

#include <iostream>
#include <exception>

#include <assert.h>

LegoOp *
LegoOp::Construct(std::istringstream &is)
{
    std::string type;
    is >> type;
    LegoOp *op = NULL;
    if (type == "CreateBrick") {
        op = new LegoOpCreateBrick();
    } else if (type == "ModifyPosition") {
        op = new LegoOpModifyPosition();
    } else {
        std::cerr << "FATAL\n";
        throw std::exception();
    }

    op->Deserialize(is);
    return op;
}

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
LegoOp::Deserialize(std::istringstream &is)
{
}

void
LegoOpCreateBrick::Serialize(std::ostringstream &os)
{
    LegoOp::Serialize(os);

    os << " " << _position << " " << _size << " " << _orientation << " " << _color << "\n";
}

void
LegoOpCreateBrick::Deserialize(std::istringstream &is)
{
    LegoOp::Deserialize(is);

    is >> _position;
    is >> _size;
    is >> _orientation;
    is >> _color;
}

void
LegoOpModifyPosition::Serialize(std::ostringstream &os)
{
    LegoOp::Serialize(os);

    os << " " << _brick << _position;
}

void
LegoOpModifyPosition::Deserialize(std::istringstream &is)
{
    LegoOp::Deserialize(is);

    is >> _brick;
    is >> _position;
}
