#include "LegoBrick.h"
#include "LegoOps.h"
#include "LegoTransactionMgr.h"

#include <vector>

bool
LegoBrick::SetPosition(const MfVec3i &position)
{
    std::vector<LegoOp*> ops;
    ops.push_back(new LegoOpModifyPosition(this, position));
    if (_xaMgr->Execute(ops)) {
        _position = position;
        return true;
    } else {
        return false;
    }
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
