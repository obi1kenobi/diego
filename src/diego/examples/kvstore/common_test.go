package kvstore

import "testing"
import "strconv"
import "math/rand"
import "diego/resolver"
import "diego/tests"
import "diego/types"

const trailingDistance = 50
const randomKeyspaceSize = 25

func setup()(*resolver.Resolver, *kvStore) {
  return makeResolver(), makeState().(*kvStore)
}

func makeResolver()*resolver.Resolver {
  return resolver.CreateResolver(makeState, trailingDistance, "")
}

func generateRandomData(fns []func(*rand.Rand)(types.Transaction, tests.TransactionResult),
                        rnd *rand.Rand) tests.TestDataItem {
  choice := rnd.Intn(len(fns))
  op, checkType := fns[choice](rnd)
  return tests.MakeTestDataItem(op, checkType, nil)
}

func generateKey(rnd *rand.Rand) string {
  return strconv.Itoa(rnd.Intn(randomKeyspaceSize))
}

func generateValue(rnd *rand.Rand) string {
  return generateKey(rnd)
}

func generateRandomLwwSet(rnd *rand.Rand)(types.Transaction, tests.TransactionResult) {
  // set the transaction id to 0 because it will be set dynamically during the test
  return &lwwSetOp{OpCore{0,
                          generateKey(rnd),
                          generateValue(rnd)}}, tests.Success
}

func generateRandomPessimisticSet(rnd *rand.Rand)(types.Transaction, tests.TransactionResult) {
  // set the transaction id to 0 because it will be set dynamically during the test
  return &pessimisticSetOp{OpCore{0,
                                  generateKey(rnd),
                                  generateValue(rnd)}}, tests.NoCheck
}

func generateRandomTestAndSet(rnd *rand.Rand)(types.Transaction, tests.TransactionResult) {
  // set the transaction id to 0 because it will be set dynamically during the test
  return &testAndSetOp{OpCore{0,
                              generateKey(rnd),
                              generateValue(rnd)}}, tests.NoCheck
}

func generateRandomAppend(rnd *rand.Rand)(types.Transaction, tests.TransactionResult) {
  // set the transaction id to 0 because it will be set dynamically during the test
  return &appendOp{OpCore{0,
                          generateKey(rnd),
                          generateValue(rnd)}}, tests.NoCheck
}

func makeTestData(data []tests.TestDataItem, rnd *rand.Rand) {
  fns := []func(*rand.Rand)(types.Transaction, tests.TransactionResult) { generateRandomLwwSet,
                                                                             generateRandomPessimisticSet,
                                                                             generateRandomAppend,
                                                                             generateRandomTestAndSet }
  for i := 0; i < len(data); i++ {
    data[i] = generateRandomData(fns, rnd)
  }
}

func stateEquals(a, b types.State)bool {
  kv := a.(*kvStore)
  kv2 := b.(*kvStore)
  return kv.Equals(kv2)
}

func keyValuePredicate(key string, value string) func(*testing.T, types.State)bool {
  return func(t *testing.T, s types.State)bool {
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
