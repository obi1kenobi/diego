#include "LegoBrick.h"
#include "LegoOps.h"
#include "LegoTransaction.h"
#include "LegoTransactionMgr.h"

#include <vector>

LegoBrick::~LegoBrick()
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeDeleteBrickOp(GetID()));
    _xaMgr->Execute(xa);
}

bool
LegoBrick::SetPosition(const MfVec3i &position)
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeModifyPositionOp(GetID(), position));
    bool success = _xaMgr->Execute(xa);
    return success;
}

bool
LegoBrick::SetSize(const MfVec3i &size)
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeModifySizeOp(GetID(), size));
    bool success = _xaMgr->Execute(xa);
    return success;
}

bool
LegoBrick::SetOrientation(Orientation orientation)
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeModifyOrientationOp(GetID(), orientation));
    bool success = _xaMgr->Execute(xa);
    return success;
}

bool
LegoBrick::SetColor(const MfVec3f &color)
{
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeModifyColorOp(GetID(), color));
    bool success = _xaMgr->Execute(xa);
    return success;
}
