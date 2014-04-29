package tests

import "testing"
import "diego/resolver"
import "diego/debug"

/*
TestDataItem -
  ops = Transactions to submit to the resolver
  submitSuccess = whether we expect the resolver to accept each transaction
  localStatePredicate = function that determines whether the local state is correct after the transaction is applied
*/
type TestDataItem struct {
  op resolver.Transaction
  submitSuccess bool
  localStatePredicate func(*testing.T, resolver.State)
}

/*
MakeTestDataItem - factory method for TestDataItem. Makes the predicate a no-op if one is not specified.
*/
func MakeTestDataItem(op resolver.Transaction, submitSuccess bool,
                      localStatePredicate func(*testing.T, resolver.State))TestDataItem {

  if localStatePredicate == nil {
    localStatePredicate = func(*testing.T, resolver.State) {};
  }
  return TestDataItem{op, submitSuccess, localStatePredicate}
}

func expectConvergedState(t *testing.T, rs *resolver.Resolver, s resolver.State,
                          equals func(resolver.State, resolver.State)bool) {
  callback := func(state resolver.State) {
    if !equals(s, state) {
      t.Fatalf("State mismatch: local=%s; remote=%s", debug.Stringify(s), debug.Stringify(state))
    }
  }
  rs.CurrentState(callback)
}

func expectSubmitSuccess(t *testing.T, rs *resolver.Resolver,
                         op resolver.Transaction, s resolver.State) {
  ok, newT := rs.SubmitTransaction(op)
  if !ok {
    t.Errorf("Transaction %s failed to apply on server", debug.Stringify(op))
  }
  ok, _ = s.Apply(newT)
  s.SetId(newT.Id() + 1)
  if !ok {
    t.Errorf("Transaction response %s failed to apply locally", debug.Stringify(newT))
  }
}

func expectSubmitFailure(t *testing.T, rs *resolver.Resolver,
                         op resolver.Transaction) {
  ok, _ := rs.SubmitTransaction(op)
  if ok {
    t.Errorf("Transaction %s applied on server when it shouldn't have", debug.Stringify(op))
  }
}

func expectCurrentStateId(t *testing.T, rs *resolver.Resolver, expid int64) {
  id := rs.CurrentStateId()
  if id != expid {
    t.Errorf("Expected current state id %d != received id %d", expid, id)
  }
}

func expectNoNewTransactions(t *testing.T, rs *resolver.Resolver, expid int64) {
  expectCurrentStateId(t, rs, expid)

  id, slice := rs.TransactionsSinceId(expid)
  if id != expid {
    t.Errorf("Expected current state id %d != received id %d", expid, id)
  }
  if len(slice) != 0 {
    t.Errorf("Expected no new transactions, but found %d", len(slice))
  }
}
