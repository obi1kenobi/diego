package kvstore

import "testing"
import "diego/resolver"
import "diego/debug"

func setup()(*resolver.Resolver, *kvStore) {
  return resolver.CreateResolver(makeState, 50), makeState().(*kvStore)
}

func expectSubmitSuccess(t *testing.T, rs *resolver.Resolver,
                         op resolver.Transaction, opType string, kv *kvStore) {
  ok, newT := rs.SubmitTransaction(op)
  if !ok {
    t.Fatalf("%s %s failed to apply", opType, debug.Stringify(op))
  }
  ok, _ = kv.Apply(newT)
  kv.id = newT.Id() + 1
  if !ok {
    t.Fatalf("%s response %s failed to apply", opType, debug.Stringify(newT))
  }
}

func expectSubmitFailure(t *testing.T, rs *resolver.Resolver,
                         op resolver.Transaction, opType string, kv *kvStore) {
  ok, _ := rs.SubmitTransaction(op)
  if ok {
    t.Fatalf("%s %s applied when it shouldn't have", opType, debug.Stringify(op))
  }
}

func expectKeyValue(t *testing.T, kv *kvStore, key string, value string) {
  v, ok := kv.data[key]
  if !ok {
    t.Fatalf("Expected key-value %s -> %s was missing", key, value)
  }
  if v != value {
    t.Fatalf("Expected %s -> %s, got %s -> %s", key, value, key, v)
  }
}

func expectCurrentStateId(t *testing.T, rs *resolver.Resolver, expid int64) {
  id := rs.CurrentStateId()
  if id != expid {
    t.Fatalf("Expected current state id %d != received id %d", expid, id)
  }
}

func expectNoNewTransactions(t *testing.T, rs *resolver.Resolver, expid int64) {
  expectCurrentStateId(t, rs, expid)

  id, slice := rs.TransactionsSinceId(expid)
  if id != expid {
    t.Fatalf("Expected current state id %d != received id %d", expid, id)
  }
  if len(slice) != 0 {
    t.Fatalf("Expected no new transactions, but found %d", len(slice))
  }
}

func expectConvergedState(t *testing.T, rs *resolver.Resolver, kv *kvStore) {
  callback := func(state resolver.State) {
    kv2 := state.(*kvStore)
    if !kv.Equals(kv2) {
      t.Fatalf("State mismatch: kv=%s; kv2=%s", debug.Stringify(kv), debug.Stringify(kv2))
    }
  }
  rs.CurrentState(callback)
}

func TestLwwSet(t *testing.T) {
  rs, s := setup()

  expectNoNewTransactions(t, rs, s.id)

  lww := &lwwSetOp{0, "a", "b"}
  expectSubmitSuccess(t, rs, lww, "LwwSetOp", s)
  expectKeyValue(t, s, "a", "b")
  expectNoNewTransactions(t, rs, s.id)

  lww = &lwwSetOp{0, "a", "c"}
  expectSubmitSuccess(t, rs, lww, "LwwSetOp", s)
  expectKeyValue(t, s, "a", "c")
  expectNoNewTransactions(t, rs, s.id)

  lww = &lwwSetOp{0, "a", "abcd"}
  expectSubmitSuccess(t, rs, lww, "LwwSetOp", s)
  expectKeyValue(t, s, "a", "abcd")
  expectNoNewTransactions(t, rs, s.id)

  lww = &lwwSetOp{3, "a", "def"}
  expectSubmitSuccess(t, rs, lww, "LwwSetOp", s)
  expectKeyValue(t, s, "a", "def")
  expectNoNewTransactions(t, rs, s.id)
  expectConvergedState(t, rs, s)
}

func TestOptimisticSet(t *testing.T) {
  rs, s := setup()

  expectNoNewTransactions(t, rs, s.id)

  opt := &optimisticSetOp{0, "a", "b"}
  expectSubmitSuccess(t, rs, opt, "OptimisticSetOp", s)
  expectKeyValue(t, s, "a", "b")
  expectNoNewTransactions(t, rs, s.id)

  opt = &optimisticSetOp{0, "a", "c"}
  expectSubmitFailure(t, rs, opt, "OptimisticSetOp", s)
  expectKeyValue(t, s, "a", "b")
  expectNoNewTransactions(t, rs, s.id)

  opt = &optimisticSetOp{1, "a", "c"}
  expectSubmitSuccess(t, rs, opt, "OptimisticSetOp", s)
  expectKeyValue(t, s, "a", "c")
  expectNoNewTransactions(t, rs, s.id)

  opt = &optimisticSetOp{2, "a", "abcd"}
  expectSubmitSuccess(t, rs, opt, "OptimisticSetOp", s)
  expectKeyValue(t, s, "a", "abcd")
  expectNoNewTransactions(t, rs, s.id)
  expectConvergedState(t, rs, s)
}

func TestAppend(t *testing.T) {
  rs, s := setup()

  expectNoNewTransactions(t, rs, s.id)

  app := &appendOp{0, "a", "b"}
  expectSubmitSuccess(t, rs, app, "AppendOp", s)
  expectKeyValue(t, s, "a", "b")
  expectNoNewTransactions(t, rs, s.id)

  app = &appendOp{0, "a", "c"}
  expectSubmitSuccess(t, rs, app, "AppendOp", s)
  expectKeyValue(t, s, "a", "bc")
  expectNoNewTransactions(t, rs, s.id)

  app = &appendOp{0, "a", "d"}
  expectSubmitSuccess(t, rs, app, "AppendOp", s)
  expectKeyValue(t, s, "a", "bcd")
  expectNoNewTransactions(t, rs, s.id)

  app = &appendOp{3, "a", "ef"}
  expectSubmitSuccess(t, rs, app, "AppendOp", s)
  expectKeyValue(t, s, "a", "bcdef")
  expectNoNewTransactions(t, rs, s.id)
  expectConvergedState(t, rs, s)
}

func TestFlipflopAdd(t *testing.T) {
  rs, s := setup()

  expectNoNewTransactions(t, rs, s.id)

  ff := &flipflopAddOp{0, "a", 0, 2}
  expectSubmitSuccess(t, rs, ff, "FlipflopAddOp", s)
  expectKeyValue(t, s, "a", "2")
  expectNoNewTransactions(t, rs, s.id)

  ff = &flipflopAddOp{1, "a", 1, 1}
  expectSubmitSuccess(t, rs, ff, "FlipflopAddOp", s)
  expectKeyValue(t, s, "a", "1")
  expectNoNewTransactions(t, rs, s.id)

  ff = &flipflopAddOp{1, "a", 1, 3}
  expectSubmitSuccess(t, rs, ff, "FlipflopAddOp", s)
  expectKeyValue(t, s, "a", "4")
  expectNoNewTransactions(t, rs, s.id)

  ff = &flipflopAddOp{0, "a", 0, 4}
  expectSubmitSuccess(t, rs, ff, "FlipflopAddOp", s)
  expectKeyValue(t, s, "a", "0")
  expectNoNewTransactions(t, rs, s.id)
  expectConvergedState(t, rs, s)
}
