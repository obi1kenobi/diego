#ifndef LEGO_TRANSACTION_MGR_H
#define LEGO_TRANSACTION_MGR_H

#include <vector>

class LegoOp;
class LegoTransaction;
class LegoUniverse;

class LegoTransactionMgr {
  public:
    LegoTransactionMgr(LegoUniverse *universe);

    bool Execute(const LegoTransaction &xa);

  private:
    bool _SendToServer(const LegoTransaction &xa, 
                       std::vector<LegoTransaction> *serverLog);
    void _Execute(const std::vector<LegoTransaction> &xas);

    LegoUniverse *_universe;
    uint64_t _xaIds;
};

#endif //  LEGO_TRANSACTION_MGR_H
