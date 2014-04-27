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

func TestLwwSet(t *testing.T) {
  rs, s := setup()

  lww := &lwwSetOp{0, "a", "b"}
  expectSubmitSuccess(t, rs, lww, "LwwSetOp", s)
  expectKeyValue(t, s, "a", "b")

  lww = &lwwSetOp{0, "a", "c"}
  expectSubmitSuccess(t, rs, lww, "LwwSetOp", s)
  expectKeyValue(t, s, "a", "c")

  lww = &lwwSetOp{0, "a", "abcd"}
  expectSubmitSuccess(t, rs, lww, "LwwSetOp", s)
  expectKeyValue(t, s, "a", "abcd")

  lww = &lwwSetOp{3, "a", "def"}
  expectSubmitSuccess(t, rs, lww, "LwwSetOp", s)
  expectKeyValue(t, s, "a", "def")
}

func TestOptimisticSet(t *testing.T) {
  rs, s := setup()

  opt := &optimisticSetOp{0, "a", "b"}
  expectSubmitSuccess(t, rs, opt, "OptimisticSetOp", s)
  expectKeyValue(t, s, "a", "b")

  opt = &optimisticSetOp{0, "a", "c"}
  expectSubmitFailure(t, rs, opt, "OptimisticSetOp", s)
  expectKeyValue(t, s, "a", "b")

  opt = &optimisticSetOp{1, "a", "c"}
  expectSubmitSuccess(t, rs, opt, "OptimisticSetOp", s)
  expectKeyValue(t, s, "a", "c")

  opt = &optimisticSetOp{2, "a", "abcd"}
  expectSubmitSuccess(t, rs, opt, "OptimisticSetOp", s)
  expectKeyValue(t, s, "a", "abcd")
}

func TestAppend(t *testing.T) {
  rs, s := setup()

  app := &appendOp{0, "a", "b"}
  expectSubmitSuccess(t, rs, app, "AppendOp", s)
  expectKeyValue(t, s, "a", "b")

  app = &appendOp{0, "a", "c"}
  expectSubmitSuccess(t, rs, app, "AppendOp", s)
  expectKeyValue(t, s, "a", "bc")

  app = &appendOp{0, "a", "d"}
  expectSubmitSuccess(t, rs, app, "AppendOp", s)
  expectKeyValue(t, s, "a", "bcd")

  app = &appendOp{3, "a", "ef"}
  expectSubmitSuccess(t, rs, app, "AppendOp", s)
  expectKeyValue(t, s, "a", "bcdef")
}

func TestFlipflopAdd(t *testing.T) {
  rs, s := setup()

  ff := &flipflopAddOp{0, "a", 0, 2}
  expectSubmitSuccess(t, rs, ff, "FlipflopAddOp", s)
  expectKeyValue(t, s, "a", "2")

  ff = &flipflopAddOp{1, "a", 1, 1}
  expectSubmitSuccess(t, rs, ff, "FlipflopAddOp", s)
  expectKeyValue(t, s, "a", "1")

  ff = &flipflopAddOp{1, "a", 1, 3}
  expectSubmitSuccess(t, rs, ff, "FlipflopAddOp", s)
  expectKeyValue(t, s, "a", "4")

  ff = &flipflopAddOp{0, "a", 0, 4}
  expectSubmitSuccess(t, rs, ff, "FlipflopAddOp", s)
  expectKeyValue(t, s, "a", "0")
}
