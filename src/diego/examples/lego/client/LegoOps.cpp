#include "LegoOps.h"

#include <iostream>
#include <exception>

#include <assert.h>

LegoOp *
LegoOp::Construct(std::istream &is)
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
LegoOp::Serialize(std::ostream &os)
{
    switch (_type) {
    case CREATE_BRICK: os << "CreateBrick"; break;
    case MODIFY_POSITION: os << "ModifyPosition"; break;
    case DELETE_BRICK: os << "DeleteBrick"; break;
    }
}

void
LegoOp::Deserialize(std::istream &is)
{
}

void
LegoOpCreateBrick::Serialize(std::ostream &os)
{
    LegoOp::Serialize(os);

    os << " " << _position;
    os << " " << _size;
    os << " " << _orientation;
    os << " " << _color;
    os << std::endl;
}

void
LegoOpCreateBrick::Deserialize(std::istream &is)
{
    LegoOp::Deserialize(is);

    is >> _position;
    is >> _size;
    is >> _orientation;
    is >> _color;
}

void
LegoOpModifyPosition::Serialize(std::ostream &os)
{
    LegoOp::Serialize(os);

    os << " " << _brickID;
    os << " " << _position;
    os << std::endl;
}

void
LegoOpModifyPosition::Deserialize(std::istream &is)
{
    LegoOp::Deserialize(is);

    is >> _brickID;
    is >> _position;
}
