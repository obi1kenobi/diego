package core

import "diego/resolver"
import "diego/namespace"

type diegoCore struct {
  nsManager *namespace.NamespaceManager
  trailingDistance int
  makeState func()resolver.State
}

/*
CreateDiegoCore - factory method to make a diego core object.
*/
func CreateDiegoCore(trailingDistance int, makeState func()resolver.State) *diegoCore {
  dc := new(diegoCore)
  dc.trailingDistance = trailingDistance

  if dc.trailingDistance <= 0 {
    dc.trailingDistance = resolver.DefaultTrailingDistance
  }

  dc.makeState = makeState
  dc.nsManager = namespace.CreateNamespaceManager()
  return dc
}

func (dc *diegoCore) robustGetNamespace(ns string) *resolver.Resolver {
  rs, ok := dc.nsManager.GetNamespace(ns)
  if !ok {
    rs := resolver.CreateResolver(dc.makeState, dc.trailingDistance)
    for !ok {
      ok = dc.nsManager.CreateNamespace(ns, rs)
      if !ok {
        // the create failed -- we lost the race to create a new namespace
        // try getting the namespace again
        // must try in a for-loop, because someone may have deleted it in the meantime
        rs, ok = dc.nsManager.GetNamespace(ns)
      }
    }
  }
  return rs
}

/*
SubmitTransaction - submits the specified transaction to the specified namespace.
  The namespace is created if it doesn't exist.
  Return value: transaction success + all transactions that the client hasn't seen yet

  Safe for concurrent use.
*/
func (dc *diegoCore) SubmitTransaction(ns string, t resolver.Transaction) (bool, []resolver.Transaction) {
  rs := dc.robustGetNamespace(ns)
  tid := t.Id()
  ok := rs.SubmitTransaction(t)
  _, transactions := rs.TransactionsSinceId(tid)
  return ok, transactions
}

/*
TransactionsSinceId - gets all transactions in the given namespace that happened at or
  after the specified state id. If the namespace doesn't exist, the second return argument
  will be false. See resolver.TransactionsSinceId for details.

  Safe for concurrent use.
*/
func (dc *diegoCore) TransactionsSinceId(ns string, id int64) ([]resolver.Transaction, bool) {
  rs, ok := dc.nsManager.GetNamespace(ns)
  if !ok {
    return nil, false
  }

  _, transactions := rs.TransactionsSinceId(id)
  return transactions, true
}

/*
CurrentStateId - gets the current state id in the given namespace. If the namespace doesn't exist,
  the second return argument will be false. See resolver.CurrentStateId for details.

  Safe for concurrent use.
*/
func (dc *diegoCore) CurrentStateId(ns string) (int64, bool) {
  rs, ok := dc.nsManager.GetNamespace(ns)
  if !ok {
    return 0, false
  }

  sid := rs.CurrentStateId()
  return sid, true
}

/*
CurrentState - calls the callback with the current state for the given namespace. If the namespace
  doesn't exist, the callback will never be called.

  The callback MUST NOT MODIFY the state.

  This is a blocking operation that will prevent all other operations from executing until the
  callback returns. This operation is as expensive as your callback,
  so USE AS QUICKLY AND SPARINGLY AS POSSIBLE!!!

  See resolver.CurrentState for details.

  Safe for concurrent use.
*/
func (dc *diegoCore) CurrentState(ns string, stateProcessor func(resolver.State)) {
  rs, ok := dc.nsManager.GetNamespace(ns)
  if !ok {
    return
  }
  rs.CurrentState(stateProcessor)
}
