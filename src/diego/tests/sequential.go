package tests

import "testing"
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
                       equals func(resolver.State, resolver.State)bool) {

  expectNoNewTransactions(t, rs, s.Id())

  for i := 0; i < len(data); i++ {
    if data[i].submitSuccess {
      expectSubmitSuccess(t, rs, data[i].op, s)
    } else {
      expectSubmitFailure(t, rs, data[i].op)
    }
    expectNoNewTransactions(t, rs, s.Id())
    expectConvergedState(t, rs, s, equals)
    data[i].localStatePredicate(t, s)
  }
}
