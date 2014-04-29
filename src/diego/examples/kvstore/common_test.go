package kvstore

import "testing"
import "diego/resolver"

func setup()(*resolver.Resolver, *kvStore) {
  return resolver.CreateResolver(makeState, 50), makeState().(*kvStore)
}

func stateEquals(a, b resolver.State)bool {
  kv := a.(*kvStore)
  kv2 := b.(*kvStore)
  return kv.Equals(kv2)
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
