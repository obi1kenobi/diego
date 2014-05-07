#include "LegoUniverse.h"

#include "LegoOps.h"
#include "LegoTransaction.h"

LegoUniverse::LegoUniverse(const MfVec3i &gridSize) :
    _id(0),
    _xaMgr(this),
    _gridSize(gridSize),
    _XY(gridSize[0] * gridSize[1]),
    _grid(gridSize[0] * gridSize[1] * gridSize[2], 0),
    _brickID(0)
{
}

bool
LegoUniverse::ProcessOp(const std::string &opText)
{
    std::istringstream ops(opText);
    LegoOp op(ops);
    if (!op.IsValid()) {
        return false;
    }
    LegoTransaction xa;
    xa.AddOp(op);
    bool success = _xaMgr.Execute(xa);
    return success;
}

bool
LegoUniverse::CreateBrick(const MfVec3i &position,
                          const MfVec3i &size,
                          LegoBrick::Orientation orientation,
                          const MfVec3f &color)
{
    // XXX: validate that a brick can be inserted at given location

    // Create transaction and send to server
    LegoTransaction xa;
    xa.AddOp(LegoOp::MakeCreateOp(position, size, orientation, color));
    bool success = _xaMgr.Execute(xa);
    return success;
}

void
LegoUniverse::_CreateBrick(const MfVec3i &position,
                           const MfVec3i &size,
                           LegoBrick::Orientation orientation,
                           const MfVec3f &color)
{
    // Transaction validated on server; update state
    ++_brickID;
    uint64_t brickID = _brickID;
    LegoBrick *brick = 
        new LegoBrick(&_xaMgr, brickID, position, size, orientation, color);
    _bricks.insert(_BrickMap::value_type(brickID, brick));
    for (int xs = 0; xs < size[0]; ++xs) {
        for (int ys = 0; ys < size[1]; ++ys) {
            for (int zs = 0; zs < size[2]; ++zs) {
                MfVec3i pos(position[0] + xs, position[1] + ys, position[2] + zs);
                _WriteGrid(pos, brickID);
            }
        }
    }
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

LegoBrick *
LegoUniverse::GetBrickAt(const MfVec3i &position) const
{
    size_t gridPos = _GetIndex(position);
    if (gridPos == size_t(-1)) {
        return NULL;
    } else {
        uint64_t brickID = _grid[gridPos];
        return GetBrick(brickID);
    }
}
