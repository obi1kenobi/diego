package kvstore

import "diego/debug"
import "diego/types"
import "diego/helpers"

// op that is rejected if there were any changes to the key since the last-seen state
// (optimistically assumes that there were no changes, fails if this was wrong)
type testAndSetOp struct {
  OpCore
}

func (op *testAndSetOp) Execute(s types.State)types.Transaction {
  kv := s.(*kvStore)
  kv.data[op.Key] = op.Value
  return op
}

func (op *testAndSetOp) CheckedApply(s types.State) (bool, types.Transaction) {
  success, newT := helpers.ApplyIfUpToDate(s, op, applierForExecutable)
  return success, newT
}

func (op *testAndSetOp) MakeContext(ancestor types.State) interface{} {
  return nil
}

func (op *testAndSetOp) UpdateContext(existing types.Transaction, context interface{}) bool {
  debug.Assert(false, "UpdateContext called on testAndSetOp")
  return true
}

func (op *testAndSetOp) CommutesWith(t types.Transaction, context interface{}) bool {
  // commutes with all operations that are not on the same key
  switch x := t.(type) {
  case *lwwSetOp:
    return x.Key != op.Key
  case *pessimisticSetOp:
    return x.Key != op.Key
  case *testAndSetOp:
    return x.Key != op.Key
  case *appendOp:
    return x.Key != op.Key
  case *flipflopAddOp:
    return x.Key != op.Key
  case *concatValuesOp:
    return x.ResultKey != op.Key
  default:
    debug.Assert(false, "Unknown transaction type %+v", t)
  }
  return false
}

func (op *testAndSetOp) ResolvesWith(t types.Transaction, context interface{}) (bool, types.Transaction) {
  // never resolves non-commutative operations
  return false, nil
}
