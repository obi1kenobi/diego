package kvstore

import "diego/debug"
import "diego/types"
import "diego/helpers"

// op that is rejected if not against latest state
// (pessimistically assumes that any unseen transactions taint the state)
type pessimisticSetOp struct {
  OpCore
}

func (op *pessimisticSetOp) Execute(s types.State)types.Transaction {
  kv := s.(*kvStore)
  kv.data[op.Key] = op.Value
  return op
}

func (op *pessimisticSetOp) CheckedApply(s types.State) (bool, types.Transaction) {
  success, newT := helpers.ApplyIfUpToDate(s, op, applierForExecutable)
  return success, newT
}

func (op *pessimisticSetOp) MakeContext(ancestor types.State) interface{} {
  return nil
}

func (op *pessimisticSetOp) UpdateContext(existing types.Transaction, context interface{}) {
  debug.Assert(false, "UpdateContext called on pessimisticSetOp")
}

func (op *pessimisticSetOp) CommutesWith(t types.Transaction, context interface{}) bool {
  // pessimistic, should abort if not latest = commutes with nothing
  return false
}

func (op *pessimisticSetOp) ResolvesWith(t types.Transaction, context interface{}) (bool, types.Transaction) {
  // never resolves
  return false, nil
}
