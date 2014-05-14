#include "LegoUniverse.h"

#include "LegoOps.h"
#include "LegoTransaction.h"
#include "Vec3d.h"

#include <iostream>

static MfVec3f _gColors[] = {
    MfVec3f(1, 0, 0),
    MfVec3f(0, 1, 0),
    MfVec3f(0, 0, 1),
    MfVec3f(1, 1, 0),
    MfVec3f(1, 1, 1),
};
static int _gNumColors = sizeof(_gColors) / sizeof(_gColors[0]);

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

LegoUniverse::~LegoUniverse()
{
    Clear();
}

void
LegoUniverse::SetNetworkEnabled(bool enabled)
{
    if (enabled == _xaMgr.IsNetworkEnabled()) {
        return;
    }

    _xaMgr.SetNetworkEnabled(enabled);
    if (enabled) {
        Restore();
        _xaMgr.Sync();
    } else {
        Snapshot();
    }
}

bool
LegoUniverse::IsNetworkEnabled() const 
{
    return _xaMgr.IsNetworkEnabled();
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
    bool success = _xaMgr.ExecuteOp(op);
    return success;
}

bool
LegoUniverse::_IsValid(const LegoOp &op)
{
    LegoOp::Type opType = op.GetType();
    if (opType == LegoOp::CREATE_BRICK ||
        opType == LegoOp::MODIFY_BRICK_POSITION || 
        opType == LegoOp::MODIFY_BRICK_SIZE) {
        MfVec3i pos = op.GetPosition();
        MfVec3i size = op.GetSize();
        if (opType != LegoOp::CREATE_BRICK) {
            auto it = _brickMap.find(op.GetBrickID());
            if (it == _brickMap.end()) {
                return false;
            }
            LegoBrick *brick = it->second;
            if (opType == LegoOp::MODIFY_BRICK_POSITION) {
                size = brick->GetSize();
            }
            if (opType == LegoOp::MODIFY_BRICK_SIZE) {
                pos = brick->GetPosition();
            }
        }
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
    if (opType != LegoOp::CREATE_BRICK) {
        LegoBrick *brick = GetBrick(op.GetBrickID());
        if (!brick) {
            std::cerr << "ERROR: Unknown brick id " << op.GetBrickID() << "\n";
            return false;
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
    _RecordBrick(brick);
}

void
LegoUniverse::_RecordBrick(LegoBrick *brick)
{
    int64_t brickID = brick->GetID();
    const MfVec3i &position = brick->GetPosition();
    const MfVec3i &size = brick->GetSize();

    _bricks.push_back(brick);
    _brickMap.insert(_BrickMap::value_type(brickID, brick));
    _WriteBrick(position, size, brickID);
}

void
LegoUniverse::_DestroyBrick(LegoBrick *brick)
{
    auto it = std::find(_bricks.begin(), _bricks.end(), brick);
    assert(it != _bricks.end());
    _bricks.erase(it);
    _brickMap.erase(brick->GetID());
    _selection.erase(brick->GetID());
    _WriteBrick(brick->GetPosition(), brick->GetSize(), 0);
    delete brick;
}

void
LegoUniverse::_WriteBrick(const MfVec3i &position, 
                          const MfVec3i &size, 
                          uint64_t brickID)
{
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

void
LegoUniverse::Snapshot()
{
    _snapshot.valid = true;
    _snapshot.id = _id;
    _snapshot.grid = _grid;
    _snapshot.bricks.clear();
    for (auto *brick : _bricks) {
        LegoBrick *clone = brick->_Clone();
        _snapshot.bricks.push_back(clone);
    }
}

void
LegoUniverse::Restore()
{
    if (!_snapshot.valid) {
        return;
    }

    Clear();

    _id = _snapshot.id;
    _grid = _snapshot.grid;
    for (auto *brick : _snapshot.bricks) {
        _RecordBrick(brick);
    }

    _snapshot.valid = false;
}

void
LegoUniverse::Clear()
{
    for (auto *brick : _bricks) {
        delete brick;
    }
    _bricks.clear();
    _brickMap.clear();
    _grid.assign(_gridSize[0] * _gridSize[1] * _gridSize[2], 0);
}

bool
LegoUniverse::Select(const MfVec3d &point)
{
    for (LegoBrick *brick : _bricks) {
        const MfVec3i &minPoint = brick->GetPosition();
        MfVec3i maxPoint = minPoint + brick->GetSize();
        bool inside = true;
        for (int i = 0; i < 3; ++i) {
            inside = inside && point[i] >= minPoint[i] && point[i] <= maxPoint[i];
        }
        if (inside) {
            _selection.insert(brick->GetID());
            std::cerr << "Selected brick id #" << brick->GetID() << std::endl;
            return true;
        }
    }

    return false;
}

void
LegoUniverse::ClearSelection()
{
    _selection.clear();
}

void
LegoUniverse::ModifyColorForSelectedBricks(Color color)
{
    if (color >= 0 && color < _gNumColors) {
        _xaMgr.OpenTransaction();
        for (auto brickID : _selection) {
            LegoBrick *brick = GetBrick(brickID);
            assert(brick);
            brick->SetColor(_gColors[color]);
        }
        _xaMgr.CloseTransaction();
    }
}
