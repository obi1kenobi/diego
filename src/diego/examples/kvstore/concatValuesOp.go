package kvstore

import "diego/debug"
import "diego/types"
import "diego/helpers"

// op that saves into ResultKey
// the concatenation of all Values + the values of all Keys at the specified Tid
type concatValuesOp struct {
  Tid int64
  ResultKey string
  Keys []string
  Values []string
}

func (op *concatValuesOp) Id() int64 {
  return op.Tid
}

func (op *concatValuesOp) SetId(id int64) {
  op.Tid = id
}

func (op *concatValuesOp) Execute(s types.State)types.Transaction {
  kv := s.(*kvStore)
  result := ""
  for _, s := range op.Values {
    result += s
  }
  for _, k := range op.Keys {
    result += kv.data[k]
  }
  kv.data[op.ResultKey] = result
  return op
}

func (op *concatValuesOp) CheckedApply(s types.State) (bool, types.Transaction) {
  return helpers.ApplyIfUpToDate(s, op, applierForExecutable)
}

func (op *concatValuesOp) MakeContext(ancestor types.State) interface{} {
  kv := ancestor.(*kvStore)
  mapping := make(map[string]int)
  for _, k := range op.Keys {
    mapping[k] = len(op.Values)
    op.Values = append(op.Values, kv.data[k])
  }
  op.Keys = make([]string, 0)
  return mapping
}

func (op *concatValuesOp) UpdateContext(existing types.Transaction, context interface{}) bool {
  mapping := context.(map[string]int)
  modkey := ""
  modval := ""
  isAppend := false
  switch x := existing.(type) {
  case *lwwSetOp:
    modkey = x.Key
    modval = x.Value
  case *pessimisticSetOp:
    modkey = x.Key
    modval = x.Value
  case *testAndSetOp:
    modkey = x.Key
    modval = x.Value
  case *appendOp:
    isAppend = true
    modkey = x.Key
    modval = x.Value
  case *concatValuesOp:
    _, ok := mapping[x.ResultKey]
    if ok {
      return false
    }
    return true
  case *flipflopAddOp:
    return false
  }

  index, ok := mapping[modkey]
  if ok {
    if isAppend {
      op.Values[index] += modval
    } else {
      op.Values[index] = modval
    }
  }

  return true
}

func (op *concatValuesOp) CommutesWith(t types.Transaction, context interface{}) bool {
  return true
}

func (op *concatValuesOp) ResolvesWith(t types.Transaction, context interface{}) (bool, types.Transaction) {
  // should never have to resolve
  debug.Assert(false, "ResolvesWith called on concatValuesOp")
  return false, nil
}
