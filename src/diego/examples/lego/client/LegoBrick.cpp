#include "LegoBrick.h"
#include "LegoOps.h"
#include "LegoTransaction.h"
#include "LegoTransactionMgr.h"
#include "LegoUniverse.h"

#include <vector>

LegoBrick::LegoBrick(LegoUniverse *universe,
                     int64_t id,
                     const MfVec3i &position,
                     const MfVec3i &size,
                     Orientation orientation,
                     const MfVec3f &color) :
    _universe(universe),
    _xaMgr(universe->GetTransactionMgr()),
    _id(id),
    _position(position),
    _size(size),
    _orientation(orientation),
    _color(color),
    _marked(false)
{
}

LegoBrick::~LegoBrick()
{
}

bool
LegoBrick::Destroy()
{
    LegoOp op = LegoOp::MakeDeleteBrickOp(GetID());
    bool success = _xaMgr->ExecuteOp(op);
    return success;
}

bool
LegoBrick::SetPosition(const MfVec3i &position)
{
    LegoOp op = LegoOp::MakeModifyBrickPositionOp(GetID(), position);
    bool success = _xaMgr->ExecuteOp(op);
    return success;
}

bool
LegoBrick::SetSize(const MfVec3i &size)
{
    LegoOp op = LegoOp::MakeModifyBrickSizeOp(GetID(), size);
    bool success = _xaMgr->ExecuteOp(op);
    return success;
}

bool
LegoBrick::SetOrientation(Orientation orientation)
{
    LegoOp op = LegoOp::MakeModifyBrickOrientationOp(GetID(), orientation);
    bool success = _xaMgr->ExecuteOp(op);
    return success;
}

bool
LegoBrick::SetColor(const MfVec3f &color)
{
    LegoOp op = LegoOp::MakeModifyBrickColorOp(GetID(), color);
    bool success = _xaMgr->ExecuteOp(op);
    return success;
}

LegoBrick *
LegoBrick::_Clone()
{
    return new LegoBrick(_universe, _id, _position, _size, 
                         (Orientation) _orientation, _color);
}
