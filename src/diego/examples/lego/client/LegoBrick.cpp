#include "LegoBrick.h"
#include "LegoOps.h"
#include "LegoTransaction.h"
#include "LegoTransactionMgr.h"

#include <vector>

bool
LegoBrick::SetPosition(const MfVec3i &position)
{
    LegoTransaction xa;
    xa.AddOp(LegoOpModifyPosition(this, position));
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
