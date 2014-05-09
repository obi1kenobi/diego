#ifndef __SF_NOTICE_MGR_H__
#define __SF_NOTICE_MGR_H__

#include "Singleton.h"

#include <boost/unordered_map.hpp>

#include <typeinfo>
#include <list>

class SfNotice;

class SfNoticeMgr : public SfSingleton<SfNoticeMgr>
{
  private:
    class _ProxyBase;
    typedef std::list<_ProxyBase*> _Proxies;

  public:
    class Key {
      private:
        friend class SfNoticeMgr;

        Key(_Proxies *proxies, _Proxies::iterator it) : 
            _proxies(proxies),
            _it(it)
        {
        }

        _Proxies *_proxies;
        _Proxies::iterator _it;
    };

    template <typename ListenerType, typename SenderType, typename NoticeType>
    using ListenerMethodWS = void (ListenerType::*)(SenderType *sender, const NoticeType &notice);

    template <typename ListenerType, typename NoticeType>
    using ListenerMethodNS = void (ListenerType::*)(const NoticeType &notice);

    /// Register to listen to a given notice from a given sender
    template <typename ListenerType, typename SenderType, typename NoticeType>
    Key Register(ListenerType *listener, 
                 SenderType *sender, 
                 ListenerMethodWS<ListenerType, SenderType, NoticeType> method);

    /// Register to listen to a given notice from a given sender
    template <typename ListenerType, typename NoticeType>
    Key Register(ListenerType *listener, 
                 ListenerMethodNS<ListenerType, NoticeType> method);

    /// Cancel registration to listen to a particular notice. 
    /// Have to provide a key that was returned when registering.
    void Cancel(const Key &key);

  private:
    friend class SfNotice;

    class _ProxyBase {
      public:
        virtual ~_ProxyBase() {}
        virtual void Send(void *sender, const SfNotice &notice) = 0;
    };

    template <typename ListenerType, typename SenderType, typename NoticeType>
    class _ProxyWithSender : public _ProxyBase {
      public:
        _ProxyWithSender() : _listener(NULL), _method(NULL) {}

        _ProxyWithSender(ListenerType *listener, 
                         SenderType *sender,
                         ListenerMethodWS<ListenerType, SenderType, NoticeType> method) :
            _listener(listener),
            _sender(sender),
            _method(method)
        {
        }

        virtual void Send(void *sender, const SfNotice &notice);

      private:
        ListenerType *_listener;
        SenderType *_sender;
        ListenerMethodWS<ListenerType, SenderType, NoticeType> _method;
    };

    template <typename ListenerType, typename NoticeType>
    class _ProxyNoSender : public _ProxyBase {
      public:
        _ProxyNoSender() : _listener(NULL), _method(NULL) {}

        _ProxyNoSender(ListenerType *listener, 
                       ListenerMethodNS<ListenerType, NoticeType> method) :
            _listener(listener),
            _method(method)
        {
        }

        virtual void Send(void *sender, const SfNotice &notice);

      private:
        ListenerType *_listener;
        ListenerMethodNS<ListenerType, NoticeType> _method;
    };

    typedef boost::unordered_map<const std::type_info*, _Proxies> _ProxyMap;

    template <typename SenderType>
    void _Send(SenderType *sender, 
               const SfNotice &notice,
               const std::type_info *noticeType);

    void _Send(const SfNotice &notice,
               const std::type_info *noticeType);

    _ProxyMap _proxyMap;
};

template <typename ListenerType, typename SenderType, typename NoticeType>
SfNoticeMgr::Key
SfNoticeMgr::Register(ListenerType *listener, 
                      SenderType *sender, 
                      ListenerMethodWS<ListenerType, SenderType, NoticeType> method)
{
    const std::type_info *ntype = &typeid(NoticeType);
    auto it = _proxyMap.find(ntype);
    if (it == _proxyMap.end()) {
        auto inserted = _proxyMap.insert(_ProxyMap::value_type(ntype, _Proxies()));
        it = inserted.first;
    }
    _Proxies &proxies = it->second;
    auto *proxy = 
        new _ProxyWithSender<ListenerType, SenderType, NoticeType>(
            listener, sender, method);
    return Key(&proxies, proxies.insert(proxies.end(), proxy));
}

template <typename ListenerType, typename NoticeType>
SfNoticeMgr::Key
SfNoticeMgr::Register(ListenerType *listener, 
                      ListenerMethodNS<ListenerType, NoticeType> method)
{
    const std::type_info *ntype = &typeid(NoticeType);
    auto it = _proxyMap.find(ntype);
    if (it == _proxyMap.end()) {
        auto inserted = _proxyMap.insert(_ProxyMap::value_type(ntype, _Proxies()));
        it = inserted.first;
    }
    _Proxies &proxies = it->second;
    auto *proxy = new _ProxyNoSender<ListenerType, NoticeType>(listener, method);
    return Key(&proxies, proxies.insert(proxies.end(), proxy));
}

template <typename SenderType>
void
SfNoticeMgr::_Send(SenderType *sender, 
                   const SfNotice &notice,
                   const std::type_info *noticeType)
{
    auto it = _proxyMap.find(noticeType);
    if (it == _proxyMap.end()) {
        // Nobody is listening
        return;
    }

    const _Proxies &proxies = it->second;
    for (auto *proxy : proxies) {
        proxy->Send(sender, notice);
    }
}

template <typename ListenerType, typename SenderType, typename NoticeType>
void
SfNoticeMgr::_ProxyWithSender<ListenerType, SenderType, NoticeType>::Send(void *sender, 
                                                                          const SfNotice &notice)
{
    if (sender != _sender)
        return;

    SenderType *typedSender = reinterpret_cast<SenderType*>(sender);
    const NoticeType &typedNotice = reinterpret_cast<const NoticeType&>(notice);
    (_listener->*_method)(typedSender, typedNotice);
}

template <typename ListenerType, typename NoticeType>
void
SfNoticeMgr::_ProxyNoSender<ListenerType, NoticeType>::Send(void *sender,
                                                                        const SfNotice &notice)
{
    const NoticeType &typedNotice = reinterpret_cast<const NoticeType&>(notice);
    (_listener->*_method)(typedNotice);
}

#endif // __SF_NOTICE_MGR_H__
