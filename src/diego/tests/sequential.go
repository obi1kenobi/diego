package tests

import "reflect"
import "testing"
import "testing/quick"
import "math/rand"
import "diego/resolver"
import "diego/types"

/*
RunSequentialTest - Test harness for sequential tests
  t = test object
  rs = Resolver object to use
  data = TestDataItems to use
  s = the initial types.State upon which to apply resolved transactions
  equals = function that determines if two copies of the state are the same
*/
func RunSequentialTest(t *testing.T, rs *resolver.Resolver,
                       data []TestDataItem, s types.State,
                       equals func(types.State, types.State)bool) bool {
  return runTest(t, rs, data, s, equals, false, nil)
}

func runTest(t *testing.T, rs *resolver.Resolver,
             data []TestDataItem, s types.State,
             equals func(types.State, types.State)bool,
             dynamicTransactionIds bool, rnd *rand.Rand) bool {
  success := true

  // oh how I wish Go had the &= operator...
  success = success && expectNoNewTransactions(t, rs, s.Id())

  for i := 0; i < len(data); i++ {
    if dynamicTransactionIds {
      data[i].op.SetId(s.Id())
      if rnd.Float32() >= idRandomizeProbability {
        randomizeTransactionId(data[i].op, rs.CurrentStateId() - int64(rs.TrailingDistance()), rnd)
      }
    }

    success = success && expectSubmitResult(t, rs, data[i].op, s, data[i].submitSuccess)
    if data[i].submitSuccess != SuccessLost {
      success = success && expectNoNewTransactions(t, rs, s.Id())
      success = success && expectConvergedState(t, rs, s, equals)
      success = success && data[i].localStatePredicate(t, s)
    }
  }
  return success
}

const randomTransactionCount = 70
const idRandomizeProbability = 0.8
const randomSeed = 0

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
                                  makeState func()types.State,
                                  makeTestData func([]TestDataItem, *rand.Rand),
                                  equals func(types.State, types.State)bool) {

  config := new(quick.Config)

  // uncomment the next line to do 1000 iterations of the random test instead of the default 50
  // config.MaxCount = 1000

  config.Rand = rand.New(rand.NewSource(randomSeed))
  config.Values = func (args []reflect.Value, rnd *rand.Rand) {
    x := make([]TestDataItem, randomTransactionCount)
    makeTestData(x, rnd)
    args[0] = reflect.ValueOf(makeResolver())
    args[1] = reflect.ValueOf(x)
    args[2] = reflect.ValueOf(makeState())
  }

  testFunc := makeRandomizedTest(t, config.Rand, equals)

  if err := quick.Check(testFunc, config); err != nil {
    t.Error(err)
  }
}

func makeRandomizedTest(t *testing.T, rnd *rand.Rand,
                        equals func(types.State, types.State)bool) func(rs *resolver.Resolver,
                                                                              data []TestDataItem,
                                                                              s types.State)bool {

  f := func(rs *resolver.Resolver, data []TestDataItem, s types.State)bool {
    return runTest(t, rs, data, s, equals, true, rnd)
  }
  return f
}
