#include "LegoUniverse.h"

#include "LegoOps.h"
#include "LegoTransaction.h"

LegoUniverse::LegoUniverse(const MfVec3i &gridSize) :
    _id(0),
    _xaMgr(this),
    _gridSize(gridSize),
    _XY(gridSize[0] * gridSize[1]),
    _grid(gridSize[0] * gridSize[1] * gridSize[2])
{
}

LegoBrick *
LegoUniverse::CreateBrick(const MfVec3i &position,
                          const MfVec3i &size,
                          LegoBrick::Orientation orientation,
                          const MfVec3f &color)
{
    // XXX: validate that a brick can be inserted at given location

    // Create transaction and send to server
    LegoTransaction xa;
    xa.AddOp(LegoOpCreateBrick(position, size, orientation, color));
    bool success = _xaMgr.Execute(xa);
    if (!success) {
        return NULL;
    }

    // Transaction validated on server; update state
    ++_numBricks;
    uint64_t brickID = _numBricks;
    LegoBrick *brick = 
        new LegoBrick(&_xaMgr, brickID, position, size, orientation, color);
    _bricks.insert(_BrickMap::value_type(brickID, brick));
    for (int xs = 0; xs < size[0]; ++xs) {
        for (int ys = 0; ys < size[1]; ++ys) {
            for (int zs = 0; zs < size[2]; ++zs) {
                MfVec3i pos(position[0] + xs, position[1] + ys, position[2] + zs);
                _WriteGrid(pos[0], pos[1], pos[2], brickID);
            }
        }
    }
    return brick;
}

LegoBrick *
LegoUniverse::GetBrick(uint64_t brickId) const
{
    auto it = _bricks.find(brickId);
    if (it == _bricks.end()) {
        return NULL;
    } else {
        return it->second;
    }
}
