#ifndef LEGO_TRANSACTION_MGR_H
#define LEGO_TRANSACTION_MGR_H

#include "LegoTransaction.h"

#include <string>
#include <vector>

class LegoOp;
class LegoUniverse;

class LegoTransactionMgr {
  public:
    LegoTransactionMgr(LegoUniverse *universe);

    bool Execute(const LegoTransaction &xa);

    void CatchupWithServer();

    const std::vector<LegoTransaction> & GetLog() const {
        return _xas;
    }

  private:
    void _EmitXaPrologue(std::ostream &os);
    void _EmitXaEpilogue(std::ostream &os);
    std::string _SendToServer(const LegoTransaction &xa);
    void _ParseResponse(std::istream &is,
                        std::vector<LegoTransaction> *serverLog);
    void _Execute(const std::vector<LegoTransaction> &xas);
    std::string _SendMessage(const std::string &message);
    void _SkipWhiteSpace(std::istream &input);

    LegoUniverse *_universe;
    uint64_t _xaIds;
    std::vector<LegoTransaction> _xas;
};

#endif //  LEGO_TRANSACTION_MGR_H
