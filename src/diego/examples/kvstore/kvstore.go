package kvstore

import "container/list"
import "diego/debug"
import "diego/types"
import "diego/helpers"

type OpCore struct {
  Tid int64
  Token types.RequestToken
  Key string
  Value string
}

type kvExecutable interface {
  // executes the transaction on the given state
  Execute(s types.State)types.Transaction

  // checks if it is safe to execute the transaction,
  // and if so, executes it
  CheckedApply(s types.State) (bool, types.Transaction)

  MakeContext(ancestor types.State) interface{}

  UpdateContext(existing types.Transaction, context interface{}) bool

  CommutesWith(t types.Transaction, context interface{}) bool

  ResolvesWith(t types.Transaction, context interface{}) (bool, types.Transaction)
}

func applierForExecutable(s types.State, t types.Transaction)types.Transaction {
  return t.(kvExecutable).Execute(s)
}

func makeContextForExecutable(current types.Transaction,
                              ancestorState *types.State)interface{} {
  return current.(kvExecutable).MakeContext(*ancestorState)
}

func updateContextForExecutable(current, existing types.Transaction,
                                context interface{})bool {
  return current.(kvExecutable).UpdateContext(existing, context)
}

func commutesWithForExecutable(current, existing types.Transaction,
                               context interface{})bool {
  return current.(kvExecutable).CommutesWith(existing, context)
}

func resolvesWithForExecutable(current,
                               existing types.Transaction,
                               context interface{})(bool, types.Transaction) {
  return current.(kvExecutable).ResolvesWith(existing, context)
}

func (op *OpCore) Id() int64 {
  return op.Tid
}

func (op *OpCore) SetId(id int64) {
  op.Tid = id
}

func (op *OpCore) GetToken() types.RequestToken {
  return op.Token
}

type kvStore struct {
  data map[string]string
  id int64

  resolveFn func(ancestorState *types.State, log *list.List,
                 current types.Transaction) (bool, types.Transaction)
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

func (kv *kvStore) Apply(t types.Transaction) (bool, types.Transaction) {
  switch x := t.(type) {
  case kvExecutable:
    return x.CheckedApply(kv)
  default:
    debug.Assert(false, "Unknown transaction type %s", debug.Stringify(t))
    return false, nil
  }
}

func (kv *kvStore) Resolve(ancestorState *types.State, log *list.List,
                           current types.Transaction) (bool, types.Transaction) {
  return kv.resolveFn(ancestorState, log, current)
}

func makeKVStore()*kvStore{
  result := new(kvStore)
  result.id = 0
  result.data = make(map[string]string)
  result.resolveFn = helpers.CreateManagedResolver(makeContextForExecutable,
                                                   updateContextForExecutable,
                                                   commutesWithForExecutable,
                                                   resolvesWithForExecutable)
  return result
}

func makeState()types.State {
  return makeKVStore()
}
