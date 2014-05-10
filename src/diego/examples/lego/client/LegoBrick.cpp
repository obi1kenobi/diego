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
    _color(color)
{
}

LegoBrick::~LegoBrick()
{
}

bool
LegoBrick::Destroy()
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeDeleteBrickOp(GetID()));
    bool success = _xaMgr->Execute(xa);
    return success;
}

bool
LegoBrick::SetPosition(const MfVec3i &position)
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeModifyBrickPositionOp(GetID(), position));
    bool success = _xaMgr->Execute(xa);
    return success;
}

bool
LegoBrick::SetSize(const MfVec3i &size)
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeModifyBrickSizeOp(GetID(), size));
    bool success = _xaMgr->Execute(xa);
    return success;
}

bool
LegoBrick::SetOrientation(Orientation orientation)
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeModifyBrickOrientationOp(GetID(), orientation));
    bool success = _xaMgr->Execute(xa);
    return success;
}

bool
LegoBrick::SetColor(const MfVec3f &color)
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeModifyBrickColorOp(GetID(), color));
    bool success = _xaMgr->Execute(xa);
    return success;
}
