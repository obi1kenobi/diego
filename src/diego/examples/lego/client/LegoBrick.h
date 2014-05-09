#ifndef LEGO_BRICK_H
#define LEGO_BRICK_H

#include "IntTypes.h"
#include "Vec2i.h"
#include "Vec3f.h"
#include "Vec3i.h"

class LegoUniverse;
class LegoTransactionMgr;

class LegoBrick {
  public:
    enum Orientation {
        NORTH,
        EAST,
        SOUTH,
        WEST,
    };

    bool Destroy();

    uint64_t GetID() const {
        return _id;
    }

    bool SetPosition(const MfVec3i &position);
    const MfVec3i & GetPosition() const {
        return _position;
    }

    bool SetSize(const MfVec3i &size);
    const MfVec3i & GetSize() const {
        return _size;
    }

    bool SetOrientation(Orientation orientation);
    Orientation GetOrientation() const {
        return Orientation(_orientation);
    }

    bool SetColor(const MfVec3f &color);
    const MfVec3f & GetColor() const {
        return _color;
    }

  private:
    ~LegoBrick();

    friend class LegoUniverse;
    friend class LegoTransactionMgr;

    LegoBrick(LegoUniverse *universe,
              uint64_t id,
              const MfVec3i &position,
              const MfVec3i &size,
              Orientation orientation,
              const MfVec3f &color);

    void _SetPosition(const MfVec3i &position) {
        _position = position;
    }
    void _SetSize(const MfVec3i &size) {
        _size = size;
    }
    void _SetOrientation(Orientation orientation) {
        _orientation = orientation;
    }
    void _SetColor(const MfVec3f &color) {
        _color = color;
    }

    void _Destroy() {
        delete this;
    }

    LegoUniverse *_universe;
    LegoTransactionMgr *_xaMgr;
    uint64_t _id;
    MfVec3i _position;
    MfVec3i _size;
    uint32_t _orientation;
    MfVec3f _color;
};

#endif // LEGO_BRICK_H
