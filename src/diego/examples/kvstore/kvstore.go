package kvstore

import "strconv"
import "container/list"
import "diego/resolver"
import "diego/debug"

type kvStore struct {
  data map[string]string
  id int64
}

// last-writer-wins set op
type lwwSetOp struct {
  id int64
  key string
  value string
}

func (op *lwwSetOp) Id() int64 {
  return op.id
}

// op that is rejected if not against latest state
// (pessimistically assumes that any unseen transactions taint the state)
type pessimisticSetOp struct {
  id int64
  key string
  value string
}

func (op *pessimisticSetOp) Id() int64 {
  return op.id
}

// append-to-key op
type appendOp struct {
  id int64
  key string
  value string
}

func (op *appendOp) Id() int64 {
  return op.id
}

// contrived example of an op that needs the log to get resolved
// opnum is incremented for each flipflopAddOp executed (on any key)
// if opnum is even, the value is added, otherwise, it is subtracted from the key
type flipflopAddOp struct {
  id int64
  key string
  opnum int
  value int
}

func (op *flipflopAddOp) Id() int64 {
  return op.id
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

func (kv *kvStore) applyLwwSet(x *lwwSetOp) (bool, resolver.Transaction) {
  kv.data[x.key] = x.value
  x.id = kv.id
  return true, x
}

func (kv *kvStore) applyPessimisticSet(x *pessimisticSetOp) (bool, resolver.Transaction) {
  if kv.id == x.id {
    kv.data[x.key] = x.value
    return true, x
  }

  return false, nil
}

func (kv *kvStore) applyAppend(x *appendOp) (bool, resolver.Transaction) {
  kv.data[x.key] = kv.data[x.key] + x.value
  x.id = kv.id
  return true, x
}

func (kv *kvStore) applyFlipflopAdd(x *flipflopAddOp) (bool, resolver.Transaction) {
  if kv.id != x.id {
    return false, nil
  }

  val, ok := kv.data[x.key]
  num := 0
  if ok {
    numm, oks := strconv.Atoi(val)
    num = numm  // silly Go, it doesn't let you have one existing and one new variable in assignments
    debug.Assert(oks == nil, "Bad conversion to int from %s", kv.data[x.key])
  }

  if x.opnum % 2 == 0 {
    num += x.value
  } else {
    num -= x.value
  }

  kv.data[x.key] = strconv.Itoa(num)
  return true, x
}

func (kv *kvStore) Apply(t resolver.Transaction) (bool, resolver.Transaction) {
  switch x := t.(type) {
  case *lwwSetOp:
    return kv.applyLwwSet(x)
  case *pessimisticSetOp:
    return kv.applyPessimisticSet(x)
  case *appendOp:
    return kv.applyAppend(x)
  case *flipflopAddOp:
    return kv.applyFlipflopAdd(x)
  default:
    debug.Assert(false, "Unknown transaction type %s", debug.Stringify(t))
    return false, nil
  }
}

func (op *flipflopAddOp) resolveFlipFlop(id int64, log *list.List) (bool, resolver.Transaction) {
  opnum := op.opnum
  newT := new(flipflopAddOp)
  newT.key = op.key
  newT.value = op.value
  newT.id = id

  e := log.Front()
  for e != nil && e.Value.(resolver.Transaction).Id() < op.id {
    e = e.Next()
  }

  debug.Assert(e != nil, "Ran out of log elements before reaching op number %d", op.id)

  for e != nil {
    val := e.Value
    switch val.(type) {
    case *flipflopAddOp:
      opnum++
    }
    e = e.Next()
  }

  newT.opnum = opnum
  return true, newT
}

func (kv *kvStore) Resolve(ancestorState *resolver.State, log *list.List,
                           current resolver.Transaction) (bool, resolver.Transaction) {
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
  default:
    debug.Assert(false, "Unknown transaction type %s", debug.Stringify(current))
    return false, nil
  }
}

func makeState()resolver.State {
  result := new(kvStore)
  result.id = 0
  result.data = make(map[string]string)
  return result
}
