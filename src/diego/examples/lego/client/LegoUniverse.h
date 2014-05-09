#ifndef LEGO_UNIVERSE_H
#define LEGO_UNIVERSE_H

#include "LegoBrick.h"
#include "LegoTransactionMgr.h"

#include <cassert>
#include <unordered_map>

class LegoUniverse {
  public:
    LegoUniverse(const MfVec3i &gridSize);

    uint64_t GetID() const {
        return _id;
    }

    LegoTransactionMgr * GetTransactionMgr() {
        return &_xaMgr;
    }

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

  private:
    friend class LegoTransactionMgr;

    typedef std::unordered_map<uint64_t, LegoBrick*> _BrickMap;

    void _CreateBrick(const MfVec3i &position,
                      const MfVec3i &size,
                      LegoBrick::Orientation orientation,
                      const MfVec3f &color);

    void _WriteGrid(const MfVec3i &pos, uint64_t brickID) {
        _grid[_GetIndex(pos)] = brickID;
    }

    uint64_t _ReadGrid(const MfVec3i &pos) const {
        return _grid[_GetIndex(pos)];
    }

    inline size_t _GetIndex(const MfVec3i &pos) const {
        if (pos[0] < 0 || pos[0] >= _gridSize[0] ||
            pos[1] < 0 || pos[1] >= _gridSize[1] ||
            pos[2] < 0 || pos[2] >= _gridSize[2]) {
            return size_t(-1);
        } else {
            return pos[2] * _XY + pos[1] * _gridSize[0] + pos[0];
        }
    }

    uint64_t _id;
    LegoTransactionMgr _xaMgr;
    MfVec3i _gridSize;
    size_t _XY;
    std::vector<LegoBrick*> _bricks;
    _BrickMap _brickMap;
    uint64_t _brickID;
    std::vector<uint64_t> _grid;
};

#endif // LEGO_UNIVERSE_H
