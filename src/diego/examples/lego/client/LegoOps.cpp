#include "LegoOps.h"

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
    } else if (type == "ModifyBrickPosition") {
        _type = MODIFY_BRICK_POSITION;
        is >> _brickID;
        is >> _position;
    } else if (type == "ModifyBrickSize") {
        _type = MODIFY_BRICK_SIZE;
        is >> _brickID;
        is >> _size;
    } else if (type == "ModifyBrickOrientation") {
        _type = MODIFY_BRICK_ORIENTATION;
        is >> _brickID;
        is >> _orientation;
    } else if (type == "ModifyBrickColor") {
        _type = MODIFY_BRICK_COLOR;
        is >> _brickID;
        is >> _color;
    } else if (type == "DeleteBrick") {
        _type = DELETE_BRICK;
        is >> _brickID;
    } else {
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
LegoOp::MakeModifyBrickPositionOp(int64_t brickID, const MfVec3i &position)
{
    LegoOp op(MODIFY_BRICK_POSITION);
    op._brickID = brickID;
    op._position = position;
    return op;
}

LegoOp
LegoOp::MakeModifyBrickSizeOp(int64_t brickID, const MfVec3i &size)
{
    LegoOp op(MODIFY_BRICK_SIZE);
    op._brickID = brickID;
    op._size = size;
    return op;
}

LegoOp
LegoOp::MakeModifyBrickColorOp(int64_t brickID, const MfVec3f &color)
{
    LegoOp op(MODIFY_BRICK_COLOR);
    op._brickID = brickID;
    op._color = color;
    return op;
}

LegoOp
LegoOp::MakeModifyBrickOrientationOp(int64_t brickID, LegoBrick::Orientation orientation)
{
    LegoOp op(MODIFY_BRICK_ORIENTATION);
    op._brickID = brickID;
    op._orientation = orientation;
    return op;
}

LegoOp
LegoOp::MakeDeleteBrickOp(int64_t brickID)
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
    case MODIFY_BRICK_POSITION: {
        os << "ModifyBrickPosition"; 
        os << " " << _brickID;
        os << " " << _position;
        os << std::endl;
    } break;
    case MODIFY_BRICK_SIZE: {
        os << "ModifyBrickSize"; 
        os << " " << _brickID;
        os << " " << _size;
        os << std::endl;
    } break;
    case MODIFY_BRICK_ORIENTATION: {
        os << "ModifyBrickOrientation"; 
        os << " " << _brickID;
        os << " " << _orientation;
        os << std::endl;
    } break;
    case MODIFY_BRICK_COLOR: {
        os << "ModifyBrickColor"; 
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
