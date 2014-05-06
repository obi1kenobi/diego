#ifndef LEGO_TRANSACTION_MGR_H
#define LEGO_TRANSACTION_MGR_H

#include <vector>

class LegoOp;
class LegoUniverse;

class LegoTransactionMgr {
  public:
    LegoTransactionMgr(LegoUniverse *universe);

    bool Execute(const std::vector<LegoOp*> &ops);

  private:
    LegoUniverse *_universe;
    uint64_t _ids;
};

#endif //  LEGO_TRANSACTION_MGR_H
