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
    // Assume for now we got back the same transaction
    serverLog->push_back(xa);

    return true;
}

void
LegoTransactionMgr::_Execute(const std::vector<LegoTransaction> &xas)
{
    for (const auto &xa : xas) {
        const auto &ops = xa.GetOps();
        for (const auto &op : ops) {
            switch (op.GetType()) {
            case LegoOp::CREATE_BRICK: {
                _universe->_CreateBrick(op.GetPosition(),
                                        op.GetSize(),
                                        op.GetOrientation(),
                                        op.GetColor());
            } break;
            case LegoOp::MODIFY_POSITION: break; {
                LegoBrick *brick = _universe->GetBrick(op.GetBrickID());
                brick->_SetPosition(op.GetPosition());
            } break;
            case LegoOp::MODIFY_SIZE: break;
            case LegoOp::MODIFY_ORIENTATION: break;
            case LegoOp::MODIFY_COLOR: break;
            case LegoOp::DELETE_BRICK: break;
            }
        }
        ++_xaIds;
    }
}
