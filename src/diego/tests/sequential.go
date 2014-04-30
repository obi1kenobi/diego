package tests

import "reflect"
import "testing"
import "testing/quick"
import "math/rand"
import "diego/resolver"

/*
RunSequentialTest - Test harness for sequential tests
  t = test object
  rs = Resolver object to use
  data = TestDataItems to use
  s = the initial resolver.State upon which to apply resolved transactions
  equals = function that determines if two copies of the state are the same
*/
func RunSequentialTest(t *testing.T, rs *resolver.Resolver,
                       data []TestDataItem, s resolver.State,
                       equals func(resolver.State, resolver.State)bool) bool {

  success := true

  // oh how I wish Go had the &= operator...
  success = success && expectNoNewTransactions(t, rs, s.Id())

  for i := 0; i < len(data); i++ {
    success = success && expectSubmitResult(t, rs, data[i].op, s, data[i].submitSuccess)
    success = success && expectNoNewTransactions(t, rs, s.Id())
    success = success && expectConvergedState(t, rs, s, equals)
    success = success && data[i].localStatePredicate(t, s)
  }
  return success
}

const randomTransactionCount = 50

/*
RunRandomizedSequentialTests - Test harness for randomized sequential tests
  t = test object
  makeResolver = Resolver-generating function to use
  makeState = State-generating function to use
  makeTestData = function that creates test data
  equals = function that determines if two copies of the state are the same
*/
func RunRandomizedSequentialTests(t *testing.T,
                                  makeResolver func()*resolver.Resolver,
                                  makeState func()resolver.State,
                                  makeTestData func([]TestDataItem, *rand.Rand),
                                  equals func(resolver.State, resolver.State)bool) {

  config := new(quick.Config)
  config.Values = func (args []reflect.Value, rnd *rand.Rand) {
    x := make([]TestDataItem, randomTransactionCount)
    makeTestData(x, rnd)
    args[0] = reflect.ValueOf(makeResolver())
    args[1] = reflect.ValueOf(x)
    args[2] = reflect.ValueOf(makeState())
  }

  testFunc := makeRandomizedTest(t, equals)

  if err := quick.Check(testFunc, config); err != nil {
    t.Error(err)
  }
}

func makeRandomizedTest(t *testing.T,
                        equals func(resolver.State, resolver.State)bool) func(rs *resolver.Resolver,
                                                                              data []TestDataItem,
                                                                              s resolver.State)bool {

  f := func(rs *resolver.Resolver, data []TestDataItem, s resolver.State)bool {
    return RunSequentialTest(t, rs, data, s, equals)
  }
  return f
}
