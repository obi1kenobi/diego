#include "LegoOps.h"

#include <iostream>
#include <exception>

#include <assert.h>

LegoOp::LegoOp(std::istream &is)
{
    _valid = true;
    std::string type;
    is >> type;
    LegoOp *op = NULL;
    if (type == "CreateBrick") {
        _type = CREATE_BRICK;
        is >> _position;
        is >> _size;
        is >> _orientation;
        is >> _color;
    } else if (type == "ModifyPosition") {
        _type = MODIFY_POSITION;
        is >> _brickID;
        is >> _position;
    } else if (type == "ModifySize") {
        _type = MODIFY_SIZE;
        is >> _brickID;
        is >> _size;
    } else if (type == "ModifyOrientation") {
        _type = MODIFY_ORIENTATION;
        is >> _brickID;
        is >> _orientation;
    } else if (type == "ModifyColor") {
        _type = MODIFY_COLOR;
        is >> _brickID;
        is >> _color;
    } else if (type == "DeleteBrick") {
        _type = DELETE_BRICK;
        is >> _brickID;
    } else {
        std::cerr << "Invalid op\n";
        _valid = false;
    }
}

LegoOp
LegoOp::MakeCreateOp(const MfVec3i &position,
                     const MfVec3i &size,
                     LegoBrick::Orientation orientation,
                     const MfVec3f &color)
{
    LegoOp op(CREATE_BRICK);
    op._position = position;
    op._size = size;
    op._orientation = orientation;
    op._color = color;
    return op;
}

LegoOp
LegoOp::MakeModifyPositionOp(uint64_t brickID, const MfVec3i &position)
{
    LegoOp op(MODIFY_POSITION);
    op._brickID = brickID;
    op._position = position;
    return op;
}

LegoOp
LegoOp::MakeModifySizeOp(uint64_t brickID, const MfVec3i &size)
{
    LegoOp op(MODIFY_SIZE);
    op._brickID = brickID;
    op._size = size;
    return op;
}

LegoOp
LegoOp::MakeModifyColorOp(uint64_t brickID, const MfVec3f &color)
{
    LegoOp op(MODIFY_COLOR);
    op._brickID = brickID;
    op._color = color;
    return op;
}

LegoOp
LegoOp::MakeModifyOrientationOp(uint64_t brickID, LegoBrick::Orientation orientation)
{
    LegoOp op(MODIFY_ORIENTATION);
    op._brickID = brickID;
    op._orientation = orientation;
    return op;
}

LegoOp
LegoOp::MakeDeleteBrickOp(uint64_t brickID)
{
    LegoOp op(DELETE_BRICK);
    op._brickID = brickID;
    return op;
}

void
LegoOp::Serialize(std::ostream &os) const
{
    switch (_type) {
    case CREATE_BRICK: {
        os << "CreateBrick"; 
        os << " " << _position;
        os << " " << _size;
        os << " " << _orientation;
        os << " " << _color;
        os << std::endl;
    } break;
    case MODIFY_POSITION: {
        os << "ModifyPosition"; 
        os << " " << _brickID;
        os << " " << _position;
        os << std::endl;
    } break;
    case MODIFY_SIZE: {
        os << "ModifySize"; 
        os << " " << _brickID;
        os << " " << _size;
        os << std::endl;
    } break;
    case MODIFY_ORIENTATION: {
        os << "ModifyOrientation"; 
        os << " " << _brickID;
        os << " " << _orientation;
        os << std::endl;
    } break;
    case MODIFY_COLOR: {
        os << "ModifyColor"; 
        os << " " << _brickID;
        os << " " << _color;
        os << std::endl;
    } break;
    case DELETE_BRICK: {
        os << "DeleteBrick"; 
        os << " " << _brickID;
        os << std::endl;
    } break;
    }
}
