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

    LegoBrick * CreateBrick(const MfVec3i &position,
                            const MfVec3i &size,
                            LegoBrick::Orientation orientation,
                            const MfVec3f &color);

    LegoBrick * GetBrick(uint64_t brickId) const;

  private:
    typedef std::unordered_map<uint64_t, LegoBrick*> _BrickMap;

    void _WriteGrid(int x, int y, int z, uint64_t brickID) {
        _grid[_GetIndex(x, y, z)] = brickID;
    }

    uint64_t _ReadGrid(int x, int y, int z) const {
        return _grid[_GetIndex(x, y, z)];
    }

    inline size_t _GetIndex(int x, int y, int z) const {
        assert(x >= 0);
        assert(y >= 0);
        assert(z >= 0);
        assert(x < _gridSize[0]);
        assert(y < _gridSize[1]);
        assert(z < _gridSize[2]);
        return z * _XY + y * _gridSize[0] + x;
    }

    uint64_t _id;
    LegoTransactionMgr _xaMgr;
    MfVec3i _gridSize;
    size_t _XY;
    _BrickMap _bricks;
    uint64_t _numBricks;
    std::vector<uint64_t> _grid;
};

#endif // LEGO_UNIVERSE_H
