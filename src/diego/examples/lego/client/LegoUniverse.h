#ifndef LEGO_UNIVERSE_H
#define LEGO_UNIVERSE_H

#include "LegoBrick.h"
#include "LegoTransactionMgr.h"

#include <QtCore/QMutex>

#include <cassert>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

class LegoUniverse {
  public:
    enum Color {
        RED,
        GREEN,
        BLUE,
        YELLOW,
        WHITE,
        NUM_COLORS,
    };

    static MfVec3f COLORS[NUM_COLORS];

    typedef std::unordered_set<uint64_t> SelectionMap;

    LegoUniverse(const MfVec3i &gridMin, const MfVec3i &gridMax);

    ~LegoUniverse();

    void NewUniverse();

    uint64_t GetID() const {
        return _id;
    }

    void SetNetworkEnabled(bool enabled);

    bool IsNetworkEnabled() const;

    void CatchupWithServer();

    LegoTransactionMgr * GetTransactionMgr() {
        return &_xaMgr;
    }

    bool IsValid(const LegoOp &op);

    bool ProcessOp(const std::string &op);

    bool CreateBrick(const MfVec3i &position,
                     const MfVec3i &size,
                     LegoBrick::Orientation orientation,
                     const MfVec3f &color);

    LegoBrick * GetBrick(uint64_t brickId) const;

    LegoBrick * GetBrickAt(const MfVec3i &position) const;

    const std::vector<LegoBrick*> & GetBricks() const {
        return _bricks;
    }

    // Save current state so that it can be restored.
    void Snapshot();

    // Restore from snap shot
    void Restore();

    LegoBrick * GetBrick(const MfVec3d &point);

    void Select(LegoBrick *brick);

    void ClearSelection();

    const SelectionMap & GetSelection() const {
        return _selection;
    }

    void ModifyColorForSelectedBricks(Color color);

    void ModifyPositionForSelectedBricks(const MfVec3i &delta);

  private:
    friend class LegoTransactionMgr;

    typedef std::unordered_map<uint64_t, LegoBrick*> _BrickMap;

    void _Clear();

    void _CreateBrick(const MfVec3i &position,
                      const MfVec3i &size,
                      LegoBrick::Orientation orientation,
                      const MfVec3f &color);

    void _WriteGrid(const MfVec3i &pos, uint64_t brickID) {
        size_t index = _GetIndex(pos);
        if (index != size_t(-1)) {
            _grid[index] = brickID;
        }
    }

    uint64_t _ReadGrid(const MfVec3i &pos) const {
        size_t index = _GetIndex(pos);
        if (index == size_t(-1)) {
            return size_t(-1);
        } else {
            return _grid[index];
        }
    }

    inline size_t _GetIndex(const MfVec3i &pos) const {
        if (pos[0] < _gridMin[0] || pos[0] > _gridMax[0] ||
            pos[1] < _gridMin[1] || pos[1] > _gridMax[1] ||
            pos[2] < _gridMin[2] || pos[2] > _gridMax[2]) {
            return size_t(-1);
        } else {
            int x = pos[0] - _gridMin[0];
            int y = pos[1] - _gridMin[1];
            int z = pos[2] - _gridMin[2];
            return z * _XY + y * _gridSize[0] + x;
        }
    }

    void _RecordBrick(LegoBrick *brick);

    void _DestroyBrick(LegoBrick *brick);

    void _WriteBrick(const MfVec3i &position, 
                     const MfVec3i &size, 
                     uint64_t brickID);

    bool _IsAvailable(const MfVec3i &position,
                      const MfVec3i &size,
                      uint64_t brickID = uint64_t(-1));

    struct _State {
        _State() : valid(false) {}

        bool valid;
        uint64_t id;
        std::vector<LegoBrick*> bricks;
        std::vector<uint64_t> grid;
    };

    bool _network;
    uint64_t _id;
    LegoTransactionMgr _xaMgr;
    LegoTransaction *_xa;
    MfVec3i _gridSize;
    MfVec3i _gridMin;
    MfVec3i _gridMax;
    size_t _XY;
    std::vector<LegoBrick*> _bricks;
    _BrickMap _brickMap;
    uint64_t _brickID;
    std::vector<uint64_t> _grid;
    SelectionMap _selection;

    _State _snapshot;

    QMutex _lockProcessOps;
};

#endif // LEGO_UNIVERSE_H
