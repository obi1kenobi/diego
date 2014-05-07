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
    return true;
}

bool
LegoBrick::SetOrientation(Orientation orientation)
{
    return true;
}

bool
LegoBrick::SetColor(const MfVec3f &color)
{
    return true;
}
