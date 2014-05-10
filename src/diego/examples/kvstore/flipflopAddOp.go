package kvstore

import "strconv"

import "diego/debug"
import "diego/types"
import "diego/helpers"

// contrived example of an op that needs the log to get resolved
// opnum is incremented for each flipflopAddOp executed (on any key)
// if opnum is even, the value is added, otherwise, it is subtracted from the key
type flipflopAddOp struct {
  Tid int64
  Key string
  Opnum int
  Value int
}

func (op *flipflopAddOp) Id() int64 {
  return op.Tid
}

func (op *flipflopAddOp) SetId(id int64) {
  op.Tid = id
}

func (op *flipflopAddOp) Execute(s types.State)types.Transaction {
  kv := s.(*kvStore)
  val, ok := kv.data[op.Key]
  num := 0
  if ok {
    numm, oks := strconv.Atoi(val)
    num = numm  // silly Go, it doesn't let you have one existing and one new variable in assignments
    debug.Assert(oks == nil, "Bad conversion to int from %s", kv.data[op.Key])
  }

  if op.Opnum % 2 == 0 {
    num += op.Value
  } else {
    num -= op.Value
  }

  kv.data[op.Key] = strconv.Itoa(num)
  return op
}

func (op *flipflopAddOp) CheckedApply(s types.State) (bool, types.Transaction) {
  success, newT := helpers.ApplyIfUpToDate(s, op, applierForExecutable)
  return success, newT
}

func (op *flipflopAddOp) MakeContext(ancestor types.State) interface{} {
  return nil
}

func (op *flipflopAddOp) UpdateContext(existing types.Transaction, context interface{}) bool {
  debug.Assert(false, "UpdateContext called on flipflopAddOp")
  return true
}

func (op *flipflopAddOp) CommutesWith(t types.Transaction, context interface{}) bool {
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

func (op *flipflopAddOp) ResolvesWith(t types.Transaction, context interface{}) (bool, types.Transaction) {
  switch t.(type) {
  case *flipflopAddOp:
    op.Opnum++
    return true, op
  default:
    // cannot resolve non-flipflops on the same key
    // they may have set a non-int value
    return false, nil
  }
  return false, nil
}
