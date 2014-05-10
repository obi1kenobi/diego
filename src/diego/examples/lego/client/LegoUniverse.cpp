#include "LegoUniverse.h"

#include "LegoOps.h"
#include "LegoTransaction.h"

#include <iostream>

LegoUniverse::LegoUniverse(const MfVec3i &gridSize) :
    _id(0),
    _xaMgr(this),
    _xa(NULL),
    _gridSize(gridSize),
    _XY(gridSize[0] * gridSize[1]),
    _grid(gridSize[0] * gridSize[1] * gridSize[2], 0),
    _brickID(0)
{
    _gridMin[0] = -_gridSize[0] / 2;
    if (_gridSize[0] % 2 == 0) {
        ++_gridMin[0];
    }
    _gridMin[1] = -_gridSize[0] / 2;
    if (_gridSize[0] % 2 == 1) {
        ++_gridMin[1];
    }
    _gridMin[2] = 0;

    _gridMax[0] = _gridSize[0] / 2;
    _gridMax[1] = _gridSize[0] / 2;
    _gridMax[2] = _gridSize[2] - 1;

    _xaMgr.CatchupWithServer();
}

void
LegoUniverse::CatchupWithServer()
{
    _xaMgr.CatchupWithServer();
}

bool
LegoUniverse::ProcessOp(const std::string &opText)
{
    std::istringstream ops(opText);
    LegoOp op(ops);
    if (!op.IsValid()) {
        std::cerr << "Invalid op\n";
        return false;
    }
    if (!_IsValid(op)) {
        std::cerr << "Invalid op\n";
        return false;
    }
    LegoTransaction xa;
    xa.AddOp(op);
    bool success = _xaMgr.ExecuteXa(xa);
    return success;
}

bool
LegoUniverse::_IsValid(const LegoOp &op)
{
    LegoOp::Type opType = op.GetType();
    if (opType == LegoOp::CREATE_BRICK || 
        opType == LegoOp::MODIFY_BRICK_POSITION || 
        opType == LegoOp::MODIFY_BRICK_SIZE) {
        const MfVec3i &pos = op.GetPosition();
        const MfVec3i &size = op.GetSize();
        for (int i = 0; i < 3; ++i) {
            if (pos[i] < _gridMin[i]) {
                return false;
            }
            if (pos[i] > _gridMax[i]) {
                return false;
            }
            if (pos[i] + size[i] > _gridMax[i]) {
                return false;
            }
        }
    }
    return true;
}

bool
LegoUniverse::CreateBrick(const MfVec3i &position,
                          const MfVec3i &size,
                          LegoBrick::Orientation orientation,
                          const MfVec3f &color)
{
    // XXX: validate that a brick can be inserted at given location

    // Create transaction and send to server
    LegoOp op = LegoOp::MakeCreateOp(position, size, orientation, color);
    bool success = _xaMgr.ExecuteOp(op);
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
        new LegoBrick(this, brickID, position, size, orientation, color);
    _bricks.push_back(brick);
    _brickMap.insert(_BrickMap::value_type(brickID, brick));
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
    auto it = _brickMap.find(brickId);
    if (it == _brickMap.end()) {
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
