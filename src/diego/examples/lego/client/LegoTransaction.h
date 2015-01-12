#ifndef LEGO_TRANSACTION_H
#define LEGO_TRANSACTION_H

#include "LegoOps.h"

#include <iosfwd>
#include <vector>

class LegoTransaction
{
  public:
    LegoTransaction() : _id(uint64_t(-1)) {}

    // Deserialization constructor.
    LegoTransaction(std::istream &is);

    // All ops are destroyed.
    ~LegoTransaction();

    void AddOp(const LegoOp &op) {
        _ops.push_back(op);
    }

    const std::vector<LegoOp> & GetOps() const {
        return _ops;
    }

    void Clear() {
        _ops.clear();
    }

    void SetID(uint64_t id) {
        _id = id;
    }

    uint64_t GetID() const {
        return _id;
    }

    void Serialize(std::ostream &os) const;

  private:
    uint64_t _id;
    std::vector<LegoOp> _ops;
};

#endif // LEGO_TRANSACTION_H
