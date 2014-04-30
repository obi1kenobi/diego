package tests

import "testing"
import "diego/resolver"
import "diego/debug"

/*
TransactionResult - do we expect the transaction to succeed/fail, or we simply don't care to check
*/
type TransactionResult int

/*
Success - we expect the transaction to succeed
*/
const Success TransactionResult = 1

/*
Failure - we expect the transaction to fail
*/
const Failure TransactionResult = -1

/*
NoCheck - we don't know whether the transaction will succeed or not
*/
const NoCheck TransactionResult = 0

/*
TestDataItem -
  ops = Transactions to submit to the resolver
  submitSuccess = whether we expect the resolver to accept each transaction
  localStatePredicate = function that determines whether the local state is correct after the transaction is applied
*/
type TestDataItem struct {
  op resolver.Transaction
  submitSuccess TransactionResult
  localStatePredicate func(*testing.T, resolver.State)bool
}

/*
MakeTestDataItem - factory method for TestDataItem. Makes the predicate a no-op if one is not specified.
*/
func MakeTestDataItem(op resolver.Transaction, success TransactionResult,
                      localStatePredicate func(*testing.T, resolver.State)bool)TestDataItem {

  if localStatePredicate == nil {
    localStatePredicate = func(*testing.T, resolver.State)bool {return true};
  }
  return TestDataItem{op, success, localStatePredicate}
}

func expectConvergedState(t *testing.T, rs *resolver.Resolver, s resolver.State,
                          equals func(resolver.State, resolver.State)bool) bool {
  success := true
  callback := func(state resolver.State) {
    if !equals(s, state) {
      t.Fatalf("State mismatch: local=%s; remote=%s", debug.Stringify(s), debug.Stringify(state))
      success = false
    }
  }
  rs.CurrentState(callback)
  return success
}

func expectSubmitSuccess(t *testing.T, rs *resolver.Resolver,
                         op resolver.Transaction, s resolver.State) bool {
  success := true
  ok, newT := rs.SubmitTransaction(op)
  if !ok {
    t.Errorf("Transaction %s failed to apply on server", debug.Stringify(op))
    success = false
  }
  ok, _ = s.Apply(newT)
  s.SetId(newT.Id() + 1)
  if !ok {
    t.Errorf("Transaction response %s failed to apply locally", debug.Stringify(newT))
    success = false
  }
  return success
}

func expectSubmitFailure(t *testing.T, rs *resolver.Resolver,
                         op resolver.Transaction) bool {
  ok, _ := rs.SubmitTransaction(op)
  if ok {
    t.Errorf("Transaction %s applied on server when it shouldn't have", debug.Stringify(op))
    return false
  }
  return true
}

func handleSubmit(t *testing.T, rs *resolver.Resolver,
                  op resolver.Transaction, s resolver.State) bool {
  ok, newT := rs.SubmitTransaction(op)
  if ok {
    ok, _ = s.Apply(newT)
    s.SetId(newT.Id() + 1)
    if !ok {
      t.Errorf("Transaction response %s failed to apply locally", debug.Stringify(newT))
      return false
    }
  }
  return true
}

func expectSubmitResult(t *testing.T, rs *resolver.Resolver,
                        op resolver.Transaction, s resolver.State,
                        result TransactionResult) bool {
  switch result {
  case Success:
    return expectSubmitSuccess(t, rs, op, s)
  case Failure:
    return expectSubmitFailure(t, rs, op)
  default:
    return handleSubmit(t, rs, op, s)
  }
}

func expectCurrentStateId(t *testing.T, rs *resolver.Resolver, expid int64) bool {
  id := rs.CurrentStateId()
  if id != expid {
    t.Errorf("Expected current state id %d != received id %d", expid, id)
    return false
  }
  return true
}

func expectNoNewTransactions(t *testing.T, rs *resolver.Resolver, expid int64) bool {
  success := expectCurrentStateId(t, rs, expid)

  id, slice := rs.TransactionsSinceId(expid)
  if id != expid {
    t.Errorf("Expected current state id %d != received id %d", expid, id)
    success = false
  }
  if len(slice) != 0 {
    t.Errorf("Expected no new transactions, but found %d", len(slice))
    success = false
  }
  return success
}
