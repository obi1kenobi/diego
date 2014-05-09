#ifndef LEGO_OP_H
#define LEGO_OP_H

#include "LegoBrick.h"
#include "Vec3f.h"
#include "Vec3i.h"

#include <sstream>

class LegoOp {
  public:
    enum Type {
        CREATE_BRICK,
        MODIFY_POSITION,
        MODIFY_SIZE,
        MODIFY_ORIENTATION,
        MODIFY_COLOR,
        DELETE_BRICK,
    };

    static LegoOp MakeCreateOp(const MfVec3i &position,
                               const MfVec3i &size,
                               LegoBrick::Orientation orientation,
                               const MfVec3f &color);

    static LegoOp MakeModifyPositionOp(uint64_t brickID,
                                       const MfVec3i &position);

    static LegoOp MakeModifySizeOp(uint64_t brickID,
                                   const MfVec3i &size);

    static LegoOp MakeModifyOrientationOp(uint64_t brickID, 
                                          LegoBrick::Orientation orientationt);

    static LegoOp MakeModifyColorOp(uint64_t brickID, const MfVec3f &color);

    static LegoOp MakeDeleteBrickOp(uint64_t brickID);

    // Deserialization constructor
    LegoOp(std::istream &is);

    bool IsValid() const {
        return _valid;
    }

    Type GetType() const {
        return _type;
    }

    uint64_t GetBrickID() const {
        return _brickID;
    }

    const MfVec3i & GetPosition() const {
        return _position;
    }

    const MfVec3i & GetSize() const {
        return _size;
    }

    LegoBrick::Orientation GetOrientation() const {
        return LegoBrick::Orientation(_orientation);
    }

    const MfVec3f & GetColor() const {
        return _color;
    }

    void Serialize(std::ostream &os) const;

  private:
    LegoOp(Type type) :
        _valid(true),
        _type(type) {}

    bool _valid;
    Type _type;
    uint64_t _brickID;
    MfVec3i _position;
    MfVec3i _size;
    uint32_t _orientation;
    MfVec3f _color;
};

#endif 
