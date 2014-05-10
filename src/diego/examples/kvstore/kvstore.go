package kvstore

import "strconv"
import "container/list"
import "diego/debug"
import "diego/types"

type kvStore struct {
  data map[string]string
  id int64
}

// last-writer-wins set op
type lwwSetOp struct {
  Tid int64
  Key string
  Value string
}

func (op *lwwSetOp) Id() int64 {
  return op.Tid
}

func (op *lwwSetOp) SetId(id int64) {
  op.Tid = id
}

// op that is rejected if not against latest state
// (pessimistically assumes that any unseen transactions taint the state)
type pessimisticSetOp struct {
  Tid int64
  Key string
  Value string
}

func (op *pessimisticSetOp) Id() int64 {
  return op.Tid
}

func (op *pessimisticSetOp) SetId(id int64) {
  op.Tid = id
}

// op that is rejected if there were any changes to the key since the last-seen state
// (optimistically assumes that there were no changes, fails if this was wrong)
type testAndSetOp struct {
  Tid int64
  Key string
  Value string
}

func (op *testAndSetOp) Id() int64 {
  return op.Tid
}

func (op *testAndSetOp) SetId(id int64) {
  op.Tid = id
}

// append-to-key op
type appendOp struct {
  Tid int64
  Key string
  Value string
}

func (op *appendOp) Id() int64 {
  return op.Tid
}

func (op *appendOp) SetId(id int64) {
  op.Tid = id
}

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

func (kv *kvStore) Equals(kv2 *kvStore) bool {
  if kv.id != kv2.id {
    return false
  }

  // ensure all elements in kv are in kv2
  for k, v := range kv.data {
    v2, ok := kv2.data[k]
    if !ok || v != v2 {
      return false
    }
  }

  // ensure all elements in kv2 are in kv
  for k, v2 := range kv2.data {
    v, ok := kv.data[k]
    if !ok || v != v2 {
      return false
    }
  }

  return true
}

func (kv *kvStore) SetId(id int64) {
  kv.id = id
}

func (kv *kvStore) Id() int64 {
  return kv.id
}

func (kv *kvStore) applyLwwSet(x *lwwSetOp) (bool, types.Transaction) {
  kv.data[x.Key] = x.Value
  x.Tid = kv.id
  return true, x
}

func (kv *kvStore) applyPessimisticSet(x *pessimisticSetOp) (bool, types.Transaction) {
  if kv.id == x.Tid {
    kv.data[x.Key] = x.Value
    return true, x
  }

  return false, nil
}

func (kv *kvStore) applyTestAndSet(x *testAndSetOp) (bool, types.Transaction) {
  if kv.id == x.Tid {
    kv.data[x.Key] = x.Value
    return true, x
  }

  return false, nil
}

func (kv *kvStore) applyAppend(x *appendOp) (bool, types.Transaction) {
  kv.data[x.Key] = kv.data[x.Key] + x.Value
  x.Tid = kv.id
  return true, x
}

func (kv *kvStore) applyFlipflopAdd(x *flipflopAddOp) (bool, types.Transaction) {
  if kv.id != x.Tid {
    return false, nil
  }

  val, ok := kv.data[x.Key]
  num := 0
  if ok {
    numm, oks := strconv.Atoi(val)
    num = numm  // silly Go, it doesn't let you have one existing and one new variable in assignments
    debug.Assert(oks == nil, "Bad conversion to int from %s", kv.data[x.Key])
  }

  if x.Opnum % 2 == 0 {
    num += x.Value
  } else {
    num -= x.Value
  }

  kv.data[x.Key] = strconv.Itoa(num)
  return true, x
}

func (kv *kvStore) Apply(t types.Transaction) (bool, types.Transaction) {
  switch x := t.(type) {
  case *lwwSetOp:
    return kv.applyLwwSet(x)
  case *pessimisticSetOp:
    return kv.applyPessimisticSet(x)
  case *appendOp:
    return kv.applyAppend(x)
  case *flipflopAddOp:
    return kv.applyFlipflopAdd(x)
  case *testAndSetOp:
    return kv.applyTestAndSet(x)
  default:
    debug.Assert(false, "Unknown transaction type %s", debug.Stringify(t))
    return false, nil
  }
}

func (op *flipflopAddOp) resolveFlipFlop(id int64, log *list.List) (bool, types.Transaction) {
  Opnum := op.Opnum
  newT := new(flipflopAddOp)
  newT.Key = op.Key
  newT.Value = op.Value
  newT.Tid = id

  e := log.Front()
  for e != nil && e.Value.(types.Transaction).Id() < op.Tid {
    e = e.Next()
  }

  debug.Assert(e != nil, "Ran out of log elements before reaching op number %d", op.Tid)

  for e != nil {
    val := e.Value
    switch val.(type) {
    case *flipflopAddOp:
      Opnum++
    }
    e = e.Next()
  }

  newT.Opnum = Opnum
  return true, newT
}

func (op *testAndSetOp) resolveTestAndSet(id int64, log *list.List) (bool, types.Transaction) {
  newT := new(testAndSetOp)
  newT.Key = op.Key
  newT.Value = op.Value
  newT.Tid = id

  e := log.Front()
  for e != nil && e.Value.(types.Transaction).Id() < op.Tid {
    e = e.Next()
  }

  debug.Assert(e != nil, "Ran out of log elements before reaching op number %d", op.Tid)

  for e != nil {
    val := e.Value
    modkey := ""
    switch x := val.(type) {
    case *lwwSetOp:
      modkey = x.Key
    case *pessimisticSetOp:
      modkey = x.Key
    case *testAndSetOp:
      modkey = x.Key
    case *appendOp:
      modkey = x.Key
    case *flipflopAddOp:
      modkey = x.Key
    }

    // if anyone modified the Key, abort
    if modkey == op.Key {
      return false, nil
    }

    e = e.Next()
  }

  // nobody modified the key, accept
  return true, newT
}

func (kv *kvStore) Resolve(ancestorState *types.State, log *list.List,
                           current types.Transaction) (bool, types.Transaction) {
  switch x := current.(type) {
  case *lwwSetOp:
    debug.Assert(false, "Shouldn't have to resolve lwwSetOp %s", debug.Stringify(x))
    return false, nil
  case *pessimisticSetOp:
    // always fails if not applied against current state
    return false, nil
  case *appendOp:
    debug.Assert(false, "Shouldn't have to resolve appendOp %s", debug.Stringify(x))
    return false, nil
  case *flipflopAddOp:
    return x.resolveFlipFlop(kv.id, log)
  case *testAndSetOp:
    return x.resolveTestAndSet(kv.id, log)
  default:
    debug.Assert(false, "Unknown transaction type %s", debug.Stringify(current))
    return false, nil
  }
}

func makeState()types.State {
  result := new(kvStore)
  result.id = 0
  result.data = make(map[string]string)
  return result
}
