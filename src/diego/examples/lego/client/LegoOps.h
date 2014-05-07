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
        DELETE_BRICK,
    };

    Type GetType() const {
        return _type;
    }

    virtual void Serialize(std::ostringstream &os);
    static LegoOp * Construct(std::istringstream &is);
    virtual void Deserialize(std::istringstream &is);

  protected:
    LegoOp(Type type) :
        _type(type) {}

  private:
    Type _type;
};

class LegoOpCreateBrick : public LegoOp {
  public:
    LegoOpCreateBrick(const MfVec3i &position,
                      const MfVec3i &size,
                      LegoBrick::Orientation orientation,
                      const MfVec3f &color) :
        LegoOp(CREATE_BRICK),
        _position(position),
        _size(size),
        _orientation(orientation),
        _color(color)
    {
    }

    virtual void Serialize(std::ostringstream &os);
    virtual void Deserialize(std::istringstream &is);

  private:
    // For deserialization
    friend class LegoOp;
    LegoOpCreateBrick() : LegoOp(CREATE_BRICK) {}

    MfVec3i _position;
    MfVec3i _size;
    uint32_t _orientation;
    MfVec3f _color;
};

class LegoOpModifyPosition : public LegoOp {
  public:
    LegoOpModifyPosition(LegoBrick *brick, const MfVec3i &position) :
        LegoOp(MODIFY_POSITION),
        _brick(brick->GetID()),
        _position(position)
    {
    }

    virtual void Serialize(std::ostringstream &os);
    virtual void Deserialize(std::istringstream &is);

  private:
    // For deserialization
    friend class LegoOp;
    LegoOpModifyPosition() : LegoOp(MODIFY_POSITION) {}

    uint64_t _brick;
    MfVec3i _position;
};

#endif 
