#include "LegoUniverse.h"

#include "Debug.h"
#include "LegoOps.h"
#include "LegoTransaction.h"
#include "Vec3d.h"

#include <Inventor/So.h>

#include <iostream>

MfVec3f LegoUniverse::COLORS[LegoUniverse::NUM_COLORS] = {
    MfVec3f(1, 0, 0),
    MfVec3f(0, 1, 0),
    MfVec3f(0, 0, 1),
    MfVec3f(1, 1, 0),
    MfVec3f(1, 1, 1),
};

LegoUniverse::LegoUniverse(const MfVec3i &gridMin, const MfVec3i &gridMax) :
    _id(0),
    _xaMgr(this),
    _xa(NULL),
    _gridMin(gridMin),
    _gridMax(gridMax),
    _gridSize(gridMax - gridMin + MfVec3i(1)),
    _XY(_gridSize[0] * _gridSize[1]),
    _grid(_gridSize[0] * _gridSize[1] * _gridSize[2], 0),
    _brickID(0),
    _gravitySensor(new SoOneShotSensor(&LegoUniverse::_ApplyGravityCB, this)),
    _gravity(false)
{
    _xaMgr.CatchupWithServer();
}

LegoUniverse::~LegoUniverse()
{
    _Clear();
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
    if (!IsValid(op)) {
        std::cerr << "Invalid op\n";
        return false;
    }
    bool success = _xaMgr.ExecuteOp(op);
    return success;
}

bool
LegoUniverse::IsValid(const LegoOp &op)
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
            if (!_IsAvailable(pos, size, brick->GetID())) {
                return false;
            }
        } else {
            if (!_IsAvailable(pos, size)) {
                return false;
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
    if (!_IsAvailable(position, size)) {
        return false;
    }

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
    SfDPrintf(0, "Created brick id %lu\n", brickID);
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
    SfDPrintf(0, "Destroyed brick id %lu\n", brick->GetID());
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

bool
LegoUniverse::_IsAvailable(const MfVec3i &position,
                           const MfVec3i &size,
                           uint64_t brickID)
{
    
    for (int xs = 0; xs < size[0]; ++xs) {
        for (int ys = 0; ys < size[1]; ++ys) {
            for (int zs = 0; zs < size[2]; ++zs) {
                MfVec3i pos(position[0] + xs, position[1] + ys, position[2] + zs);
                uint64_t gridBrickID = _ReadGrid(pos);
                if (brickID == uint64_t(-1)) {
                    if (gridBrickID > 0) {
                        return false;
                    }
                } else {
                    if (gridBrickID > 0 && gridBrickID != brickID) {
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
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

    _Clear();

    _id = _snapshot.id;
    _grid = _snapshot.grid;
    for (auto *brick : _snapshot.bricks) {
        _RecordBrick(brick);
    }

    _snapshot.valid = false;
}

void
LegoUniverse::_Clear()
{
    for (auto *brick : _bricks) {
        delete brick;
    }
    _bricks.clear();
    _brickMap.clear();
    _selection.clear();
    _grid.assign(_gridSize[0] * _gridSize[1] * _gridSize[2], 0);
}

LegoBrick *
LegoUniverse::GetBrick(const MfVec3d &point)
{
    for (LegoBrick *brick : _bricks) {
        const MfVec3i &minPoint = brick->GetPosition();
        MfVec3i maxPoint = minPoint + brick->GetSize();
        bool inside = true;
        for (int i = 0; i < 3; ++i) {
            inside = inside && point[i] >= minPoint[i] && point[i] <= maxPoint[i];
        }
        if (inside) {
            return brick;
        }
    }

    return NULL;
}

void
LegoUniverse::Select(LegoBrick *brick)
{
    _selection.insert(brick->GetID());
}

void
LegoUniverse::ClearSelection()
{
    _selection.clear();
}

void
LegoUniverse::ModifyColorForSelectedBricks(Color color)
{
    if (_selection.empty()) {
        return;
    }

    if (color >= 0 && color < NUM_COLORS) {
        _xaMgr.OpenTransaction();
        for (auto brickID : _selection) {
            LegoBrick *brick = GetBrick(brickID);
            assert(brick);
            brick->SetColor(LegoUniverse::COLORS[color]);
        }
        _xaMgr.CloseTransaction();
    }
}

void
LegoUniverse::ModifyPositionForSelectedBricks(const MfVec3i &delta)
{
    if (_selection.empty()) {
        return;
    }

    _xaMgr.OpenTransaction();
    for (auto brickID : _selection) {
        LegoBrick *brick = GetBrick(brickID);
        assert(brick);
        const MfVec3i &currentPosition = brick->GetPosition();
        MfVec3i newPosition = currentPosition + delta;
        brick->SetPosition(newPosition);
    }
    _xaMgr.CloseTransaction();
}

void
LegoUniverse::NewUniverse()
{
    _xaMgr.OpenTransaction();

    // Have to copy container because destroy will end up modifying _bricks
    auto bricks = _bricks;
    for (auto *brick : bricks) {
        brick->Destroy();
    }

    _xaMgr.CloseTransaction();
}

void
LegoUniverse::SetGravityEnabled(bool enabled)
{
    _gravity = enabled;
    if (_gravity) {
        _gravitySensor->schedule();
    } else {
        _gravitySensor->unschedule();
    }
}

void
LegoUniverse::_ApplyGravityCB(void *userData, SoSensor *sensor)
{
    LegoUniverse *This = reinterpret_cast<LegoUniverse*>(userData);

    if (!This->_gravity) {
        return;
    }

    This->_ApplyGravity();

    This->_gravitySensor->schedule();
}

void
LegoUniverse::_ApplyGravity()
{
    for (auto *brick : _bricks) {
        brick->ResetMark();
    }

    bool moves;
    do {
        moves = false;
        for (auto *brick : _bricks) {
            if (brick->IsMarked()) {
                continue;
            }

            const auto &position = brick->GetPosition();
            const auto &size = brick->GetSize();
            const auto &id = brick->GetID();
            if (position[2] == 0) {
                continue;
            }
            auto newPosition = position - MfVec3i(0, 0, 1);
            if (_IsAvailable(position - position, size, id)) {
                brick->SetPosition(newPosition);
                brick->Mark();
                moves = true;
            }
        }
    } while (moves);
}
