#ifndef __SF_NOTICE_H__
#define __SF_NOTICE_H__

#include "NoticeMgr.h"

#include <typeinfo>

class SfNotice
{
  public:
    virtual ~SfNotice() {}

    template <typename SenderType>
    void Send(SenderType *sender) {
        SfNoticeMgr::Get()._Send(sender, *this, &typeid(*this));
    }

    void Send() {
        SfNoticeMgr::Get()._Send(*this, &typeid(*this));
    }
};

#endif // __SF_NOTICE_H__
