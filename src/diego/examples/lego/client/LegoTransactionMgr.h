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

    void SetNetworkEnabled(bool enabled);

    bool IsNetworkEnabled() const {
        return _network;
    }

    void Sync();

    void OpenTransaction();

    void CloseTransaction();

    bool ExecuteOp(const LegoOp &op);

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
    void _ExecuteXas(const std::vector<LegoTransaction> &xas);
    bool _ExecuteXa(const LegoTransaction &xa);
    void _ExecuteXaOps(const LegoTransaction &xa);
    void _ExecuteOp(const LegoOp &op);
    std::string _SendMessage(const std::string &message);
    void _SkipWhiteSpace(std::istream &input);

    bool _network;
    LegoUniverse *_universe;
    uint64_t _xaIds;
    std::vector<LegoTransaction> _xas;
    LegoTransaction *_xa;
    LegoTransaction _offlineXa;
};

#endif //  LEGO_TRANSACTION_MGR_H
