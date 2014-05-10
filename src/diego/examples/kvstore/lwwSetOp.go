package kvstore

import "diego/debug"
import "diego/types"

// last-writer-wins set op
type lwwSetOp struct {
  OpCore
}

func (op *lwwSetOp) Execute(s types.State)types.Transaction {
  kv := s.(*kvStore)
  kv.data[op.Key] = op.Value
  op.Tid = kv.id
  return op
}

func (op *lwwSetOp) CheckedApply(s types.State) (bool, types.Transaction) {
  return true, op.Execute(s)
}

func (op *lwwSetOp) MakeContext(ancestor types.State) interface{} {
  return nil
}

func (op *lwwSetOp) UpdateContext(existing types.Transaction, context interface{}) {
  debug.Assert(false, "UpdateContext called on lwwSetOp")
}

func (op *lwwSetOp) CommutesWith(t types.Transaction, context interface{}) bool {
  // last-write-wins = commutes with everything
  return true
}

func (op *lwwSetOp) ResolvesWith(t types.Transaction, context interface{}) (bool, types.Transaction) {
  // should never have to resolve
  debug.Assert(false, "ResolvesWith called on lwwSetOp")
  return false, nil
}
