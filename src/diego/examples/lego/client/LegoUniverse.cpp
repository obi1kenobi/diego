#include "LegoUniverse.h"
#include "LegoOps.h"

LegoUniverse::LegoUniverse() :
    _id(0),
    _xaMgr(this)
{
}

LegoBrick *
LegoUniverse::CreateBrick(const MfVec3i &position,
                          const MfVec3i &size,
                          LegoBrick::Orientation orientation,
                          const MfVec3f &color)
{
    std::vector<LegoOp*> ops;
    ops.push_back(new LegoOpCreateBrick(position, size, orientation, color));
    bool success = _xaMgr.Execute(ops);
    if (success) {
        ++_numBricks;
        uint64_t brickId = _numBricks;
        return new LegoBrick(&_xaMgr, brickId, position, size, orientation, color);
    } else {
        return NULL;
    }
}
