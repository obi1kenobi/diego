#ifndef LEGO_BRICK_H
#define LEGO_BRICK_H

#include "IntTypes.h"
#include "Vec2i.h"
#include "Vec3f.h"
#include "Vec3i.h"

class LegoTransactionMgr;

class LegoBrick {
  public:
    enum Orientation {
        NORTH,
        EAST,
        SOUTH,
        WEST,
    };

    bool SetPosition(const MfVec3i &position);
    bool SetSize(const MfVec3i &size);
    bool SetOrientation(Orientation orientation);

  private:
    friend class LegoUniverse;

    LegoBrick(LegoTransactionMgr *xaMgr,
              uint64_t id,
              const MfVec3i &position,
              const MfVec3i &size,
              Orientation orientation,
              const MfVec3f &color) :
        _xaMgr(xaMgr),
        _id(id),
        _position(position),
        _size(size),
        _orientation(orientation),
        _color(color)
    {
    }

    LegoTransactionMgr *_xaMgr;
    uint64_t _id;
    MfVec3i _position;
    MfVec3i _size;
    Orientation _orientation;
    MfVec3f _color;
};

#endif // LEGO_BRICK_H
