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
        LegoOp op(is);
        _ops.push_back(op);
    }
}

LegoTransaction::~LegoTransaction()
{
}

void
LegoTransaction::Serialize(std::ostream &os) const
{
    for (const auto &op : _ops) {
        op.Serialize(os);
    }
}
