package kvstore

import "testing"
import "diego/resolver"
import "diego/tests"

func setup()(*resolver.Resolver, *kvStore) {
  return resolver.CreateResolver(makeState, 50), makeState().(*kvStore)
}

func keyValuePredicate(key string, value string) func(*testing.T, resolver.State) {
  return func(t *testing.T, s resolver.State) {
    kv := s.(*kvStore)
    v, ok := kv.data[key]
    if !ok {
      t.Errorf("Expected key-value %s -> %s was missing", key, value)
    } else if v != value {
      t.Errorf("Expected %s -> %s, got %s -> %s", key, value, key, v)
    }
  }
}

func stateEquals(a, b resolver.State)bool {
  kv := a.(*kvStore)
  kv2 := b.(*kvStore)
  return kv.Equals(kv2)
}

func TestLwwSet(t *testing.T) {
  rs, s := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(&lwwSetOp{0, "a", "b"}, true, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&lwwSetOp{0, "a", "c"}, true, keyValuePredicate("a", "c")),
    tests.MakeTestDataItem(&lwwSetOp{0, "a", "abcd"}, true, keyValuePredicate("a", "abcd")),
    tests.MakeTestDataItem(&lwwSetOp{3, "a", "def"}, true, keyValuePredicate("a", "def")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestOptimisticSet(t *testing.T) {
  rs, s := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(&optimisticSetOp{0, "a", "b"}, true, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&optimisticSetOp{0, "a", "c"}, false, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&optimisticSetOp{1, "a", "c"}, true, keyValuePredicate("a", "c")),
    tests.MakeTestDataItem(&optimisticSetOp{2, "a", "abcd"}, true, keyValuePredicate("a", "abcd")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestAppend(t *testing.T) {
  rs, s := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(&appendOp{0, "a", "b"}, true, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&appendOp{0, "a", "c"}, true, keyValuePredicate("a", "bc")),
    tests.MakeTestDataItem(&appendOp{1, "a", "d"}, true, keyValuePredicate("a", "bcd")),
    tests.MakeTestDataItem(&appendOp{3, "a", "ef"}, true, keyValuePredicate("a", "bcdef")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestFlipflopAdd(t *testing.T) {
  rs, s := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(&flipflopAddOp{0, "a", 0, 2}, true, keyValuePredicate("a", "2")),
    tests.MakeTestDataItem(&flipflopAddOp{1, "a", 1, 1}, true, keyValuePredicate("a", "1")),
    tests.MakeTestDataItem(&flipflopAddOp{1, "a", 1, 3}, true, keyValuePredicate("a", "4")),
    tests.MakeTestDataItem(&flipflopAddOp{0, "a", 0, 4}, true, keyValuePredicate("a", "0")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}
