package kvstore

import "testing"
import "strconv"
import "math/rand"
import "diego/resolver"
import "diego/tests"

const trailingDistance = 50
const randomKeyspaceSize = 50

func setup()(*resolver.Resolver, *kvStore) {
  return makeResolver(), makeState().(*kvStore)
}

func makeResolver()*resolver.Resolver {
  return resolver.CreateResolver(makeState, trailingDistance)
}

func generateRandomData(fns []func(int64, *rand.Rand)resolver.Transaction,
                        correctId int64, rnd *rand.Rand) tests.TestDataItem {
  choice := rnd.Intn(len(fns))
  op := fns[choice](correctId, rnd)
  return tests.MakeTestDataItem(op, tests.NoCheck, nil)
}

func generateKey(rnd *rand.Rand) string {
  return strconv.Itoa(rnd.Intn(randomKeyspaceSize))
}

func generateValue(rnd *rand.Rand) string {
  return generateKey(rnd)
}

func generateRandomLwwSet(correctId int64, rnd *rand.Rand)resolver.Transaction {
  return &lwwSetOp{correctId,
                   generateKey(rnd),
                   generateValue(rnd)}
}

func generateRandomPessimisticSet(correctId int64, rnd *rand.Rand)resolver.Transaction {
  return &pessimisticSetOp{correctId,
                           generateKey(rnd),
                           generateValue(rnd)}
}

func generateRandomAppend(correctId int64, rnd *rand.Rand)resolver.Transaction {
  return &appendOp{correctId,
                   generateKey(rnd),
                   generateValue(rnd)}
}

func makeTestData(data []tests.TestDataItem, rnd *rand.Rand) {
  fns := []func(int64, *rand.Rand)resolver.Transaction { generateRandomLwwSet,
                                                         generateRandomPessimisticSet,
                                                         generateRandomAppend }
  for i := 0; i < len(data); i++ {
    data[i] = generateRandomData(fns, int64(i), rnd)
  }
}

func stateEquals(a, b resolver.State)bool {
  kv := a.(*kvStore)
  kv2 := b.(*kvStore)
  return kv.Equals(kv2)
}

func keyValuePredicate(key string, value string) func(*testing.T, resolver.State)bool {
  return func(t *testing.T, s resolver.State)bool {
    kv := s.(*kvStore)
    v, ok := kv.data[key]
    if !ok {
      t.Errorf("Expected key-value %s -> %s was missing", key, value)
      return false
    } else if v != value {
      t.Errorf("Expected %s -> %s, got %s -> %s", key, value, key, v)
      return false
    }
    return true
  }
}
