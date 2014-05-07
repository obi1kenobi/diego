#ifndef LEGO_TRANSACTION_H
#define LEGO_TRANSACTION_H

#include "LegoOps.h"

#include <iosfwd>
#include <vector>

class LegoTransaction
{
  public:
    LegoTransaction() {}

    // Deserialization constructor.
    LegoTransaction(std::istream &is);

    // All ops are destroyed.
    ~LegoTransaction();

    template <typename Op>
    void AddOp(const Op &op) {
        Op *myOp = new Op(op);
        _ops.push_back(myOp);
    }

    const std::vector<LegoOp*> & GetOps() const {
        return _ops;
    }

    void Serialize(std::ostream &os) const;

  private:
    std::vector<LegoOp*> _ops;
};

#endif // LEGO_TRANSACTION_H
