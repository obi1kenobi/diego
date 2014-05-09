#ifndef LEGO_TRANSACTION_MGR_H
#define LEGO_TRANSACTION_MGR_H

#include <string>
#include <vector>

class LegoOp;
class LegoTransaction;
class LegoUniverse;

class LegoTransactionMgr {
  public:
    LegoTransactionMgr(LegoUniverse *universe);

    bool Execute(const LegoTransaction &xa);

    void CatchupWithServer();

  private:
    void _EmitXaPrologue(std::ostream &os);
    void _EmitXaEpilogue(std::ostream &os);
    std::string _SendToServer(const LegoTransaction &xa);
    bool _ParseResponse(const std::string &response,
                        std::vector<LegoTransaction> *serverLog);
    void _Execute(const std::vector<LegoTransaction> &xas);
    std::string _SendText(const std::string &text);
    void _SkipWhiteSpace(std::istream &input);

    LegoUniverse *_universe;
    uint64_t _xaIds;
};

#endif //  LEGO_TRANSACTION_MGR_H
