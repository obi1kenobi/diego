#ifndef __SF_NOTICES_H__
#define __SF_NOTICES_H__

#include "Notice.h"

class LegoBrick;

class LegoBrickChangedNotice : public SfNotice 
{
  public:
    LegoBrickChangedNotice(LegoBrick *brick) : 
        _brick(brick)
    {
    }

    LegoBrick * GetBrick() const {
        return _brick;
    }

  private:
    LegoBrick *_brick;
};

class LegoBricksChangedNotice : public SfNotice 
{
};

#endif // __SF_NOTICE_H__
