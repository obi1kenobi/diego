#include "LegoTransactionMgr.h"
#include "LegoOps.h"
#include "LegoTransaction.h"
#include "LegoUniverse.h"

#include <cassert>
#include <iostream>
#include <sstream>

LegoTransactionMgr::LegoTransactionMgr(LegoUniverse *universe) : 
    _universe(universe),
    _xaIds(1)
{
}

bool
LegoTransactionMgr::Execute(const LegoTransaction &xa)
{
    std::vector<LegoTransaction> serverLog;
    bool success = _SendToServer(xa, &serverLog);
    _Execute(serverLog);
    return success;
}

bool
LegoTransactionMgr::_SendToServer(const LegoTransaction &xa,
                                  std::vector<LegoTransaction> *serverLog)
{
    // Assign transaction id
    uint64_t xaID = _xaIds;

    // Serialize xa
    std::ostringstream os;
    os << _universe->GetID() << " " << xaID << "\n";
    xa.Serialize(os);
    os << "*";

    // XXX: send over wire
    std::string msg = os.str();
    std::cout << "Transaction sent:\n" << os.str() << std::endl;

    // XXX: receive response

    return true;
}

void
LegoTransactionMgr::_Execute(const std::vector<LegoTransaction> &xas)
{
    for (const auto &xa : xas) {
        const auto &ops = xa.GetOps();
        for (auto *op : ops) {
            switch (op->GetType()) {
            case LegoOp::CREATE_BRICK: break;
            case LegoOp::MODIFY_POSITION: break; {
                LegoOpModifyPosition *modifyOp =
                    dynamic_cast<LegoOpModifyPosition*>(op);
                assert(modifyOp);
                LegoBrick *brick = _universe->GetBrick(modifyOp->GetBrickID());
                brick->_SetPosition(modifyOp->GetPosition());
            } break;
            case LegoOp::DELETE_BRICK: break;
            }
        }
        ++_xaIds;
    }
}
