#include "LegoTransaction.h"

#include <istream>
#include <ostream>

LegoTransaction::LegoTransaction(std::istream &is)
{
    std::string buffer;
    while (true) {
        int next = is.peek();
        if (next == '*') {
            break;
        }
        LegoOp *op = LegoOp::Construct(is);
        _ops.push_back(op);
    }
}

LegoTransaction::~LegoTransaction()
{
    for (auto *op : _ops) {
        delete op;
    }
}

void
LegoTransaction::Serialize(std::ostream &os) const
{
    for (auto *op : _ops) {
        op->Serialize(os);
    }
}
