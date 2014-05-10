package kvstore

import "diego/debug"
import "diego/types"

// append-to-key op
type appendOp struct {
  OpCore
}

func (op *appendOp) Execute(s types.State)types.Transaction {
  kv := s.(*kvStore)
  kv.data[op.Key] = kv.data[op.Key] + op.Value
  op.Tid = kv.id
  return op
}

func (op *appendOp) CheckedApply(s types.State) (bool, types.Transaction) {
  return true, op.Execute(s)
}

func (op *appendOp) MakeContext(ancestor types.State) interface{} {
  return nil
}

func (op *appendOp) UpdateContext(existing types.Transaction, context interface{}) bool {
  debug.Assert(false, "UpdateContext called on appendOp")
  return true
}

func (op *appendOp) CommutesWith(t types.Transaction, context interface{}) bool {
  // always commutes
  return true
}

func (op *appendOp) ResolvesWith(t types.Transaction, context interface{}) (bool, types.Transaction) {
  // should never have to resolve
  debug.Assert(false, "ResolvesWith called on appendOp")
  return false, nil
}
