#ifndef __SF_NOTICES_H__
#define __SF_NOTICES_H__

#include "Notice.h"

#include <string>

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

class LegoConflictNotice : public SfNotice
{
};

class LegoTransactionProcessed : public SfNotice {
  public:
    LegoTransactionProcessed(const std::string &xa) :
        _xa(xa) {}

    const std::string & Get() const {
        return _xa;
    }

  private:
    std::string _xa;
};

#endif // __SF_NOTICE_H__
