#include "NoticeMgr.h"

SF_INSTANTIATE_SINGLETON(SfNoticeMgr);

void
SfNoticeMgr::Cancel(const Key &key)
{
    delete *key._it;
    key._proxies->erase(key._it);
}

void
SfNoticeMgr::_Send(const SfNotice &notice,
                   const std::type_info *noticeType)
{
    auto it = _proxyMap.find(noticeType);
    if (it == _proxyMap.end()) {
        // Nobody is listening
        return;
    }

    const _Proxies &proxies = it->second;
    for (auto *proxy : proxies) {
        proxy->Send(NULL, notice);
    }
}
