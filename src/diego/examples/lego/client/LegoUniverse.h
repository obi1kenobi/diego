#ifndef LEGO_UNIVERSE_H
#define LEGO_UNIVERSE_H

#include "LegoBrick.h"
#include "LegoTransactionMgr.h"

#include <unordered_map>

class LegoUniverse {
  public:
    LegoUniverse();

    uint64_t GetID() const {
        return _id;
    }

    LegoBrick * CreateBrick(const MfVec3i &position,
                            const MfVec3i &size,
                            LegoBrick::Orientation orientation,
                            const MfVec3f &color);

  private:
    uint64_t _id;
    std::unordered_map<uint64_t, LegoBrick> _bricks;
    uint64_t _numBricks;
    MfVec3i _grid;
    LegoTransactionMgr _xaMgr;
};

#endif // LEGO_UNIVERSE_H
