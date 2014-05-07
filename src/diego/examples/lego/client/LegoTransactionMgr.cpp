#include "LegoTransactionMgr.h"
#include "LegoOps.h"
#include "LegoUniverse.h"

#include <iostream>
#include <sstream>

LegoTransactionMgr::LegoTransactionMgr(LegoUniverse *universe) : 
    _universe(universe),
    _ids(0)
{
}

bool
LegoTransactionMgr::Execute(const std::vector<LegoOp*> &ops)
{
    // Get transaction id
    uint64_t xaID = _ids;

    std::ostringstream os;

    os << _universe->GetID() << " " << xaID << "\n";

    for (auto *op : ops) {
        op->Serialize(os);
        delete op;
    }

    os << "*";

    // XXX: send over wire
    // XXX: receive response
    std::cout << "Transaction sent:\n" << os.str();

    return true;
}
