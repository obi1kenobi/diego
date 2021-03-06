package resolver

import "sync"
import "container/list"
import "diego/debug"
import "diego/durable"
import "diego/types"

/*
The default maximum distance between the current and the trailing state.
*/
const DefaultTrailingDistance = 500000

/*
Resolver - The main resolver object that this framework provides.
*/
type Resolver struct {
  mu sync.RWMutex
  currentState types.State
  trailingState types.State
  log *list.List // in order of oldest transaction (Front) to newest transaction (Back)
  transactionIdLookup map[int64]*list.Element
  trailingDistance int
  durableLogger *durable.TransactionLogger
  requestTokens map[types.RequestToken]*list.Element
  closed bool
}

/*
CreateResolver - Factory method to make a resolver based on a State-generating function.
  If durablePath is "", no durable copy of transactions will be recorded. Otherwise, the path must
  point to a directory which is either empty or the location of old transaction records.
*/
func CreateResolver(makeState func()types.State, trailingDistance int, durablePath string) *Resolver {
  rs := new(Resolver)
  rs.currentState = makeState()
  rs.trailingState = makeState()
  rs.currentState.SetId(0)
  rs.trailingState.SetId(0)
  rs.trailingDistance = trailingDistance
  rs.log = new(list.List)
  rs.transactionIdLookup = make(map[int64]*list.Element)
  rs.requestTokens = make(map[types.RequestToken]*list.Element)

  if durablePath != "" {
    rs.durableLogger = durable.CreateTransactionLogger(durablePath)

    // apply any transaction that exist in the log
    transactionProcessor := func(t types.Transaction) {
      rs.submitTransactionLockless(t, false)
    }
    rs.durableLogger.ReadAll(transactionProcessor)
  }

  return rs
}

func (rs *Resolver) appendTransaction(t types.Transaction, appendToDurableLog bool) {
  debug.Assert(rs.log.Len() <= rs.trailingDistance,
               "Log length %d > trailing distance %d",
               rs.log.Len(), rs.trailingDistance)

  if rs.log.Len() == rs.trailingDistance {
    sid := rs.trailingState.Id()
    oldT := rs.log.Front().Value.(types.Transaction)
    oldtrid := oldT.Id()

    debug.Assert(sid == oldtrid,
                 "Id mismatch: sid %d != oldtrid %d",
                 sid, oldtrid)

    // the log is full, first apply a transaction to the trailing state
    ok, _ := rs.trailingState.Apply(oldT)
    rs.trailingState.SetId(sid + 1)

    debug.Assert(ok,
                 "Failed to apply transaction %+v to trailing state %+v",
                 rs.log.Front().Value, rs.trailingState)

    delete(rs.transactionIdLookup, oldtrid)
    delete(rs.requestTokens, oldT.GetToken())
    rs.log.Remove(rs.log.Front())
  }

  if appendToDurableLog && rs.durableLogger != nil {
    rs.durableLogger.Append(t)
  }

  rs.log.PushBack(t)
  rs.transactionIdLookup[t.Id()] = rs.log.Back()
  rs.requestTokens[t.GetToken()] = rs.log.Back()
}

func assertRecentTransaction(s *types.State, t types.Transaction) {
  debug.Assert((*s).Id() == t.Id(),
               "Transaction %+v not recent relative to state %+v",
               t, s)
}

func (rs *Resolver) transactionSuccess(t types.Transaction, appendToDurableLog bool) (bool, types.Transaction) {
  assertRecentTransaction(&rs.currentState, t)

  rs.currentState.SetId(rs.currentState.Id() + 1)
  rs.appendTransaction(t, appendToDurableLog)
  return true, t
}

/*
Close - close the resolver and do cleanup
  calling any function except Close on a closed resolver will return nil/error/false values,
  as appropriate.
*/
func (rs *Resolver) Close() {
  rs.mu.Lock()
  defer rs.mu.Unlock()

  if rs.closed {
    return
  }

  rs.closed = true
  if rs.durableLogger != nil {
    rs.durableLogger.Close()
  }
}

/*
TransactionsSinceId - Returns the current state id and a slice of all transactions
  at or after the specified id to the current state. The expectation is that this function
  will not be called with an id argument that is "too old".

  If the id is smaller than the id of the trailing state, no Transactions are returned.
  If the id is not smaller than the id of the trailing state, but is still "old", then this
    function call may be expensive.
*/
func (rs *Resolver) TransactionsSinceId(id int64) (int64, []types.Transaction) {
  rs.mu.RLock()
  defer rs.mu.RUnlock()

  if rs.closed {
    return 0, nil
  }

  return rs.transactionsSinceIdLockless(id)
}

func (rs *Resolver) transactionsSinceIdLockless(id int64) (int64, []types.Transaction) {

  sid := rs.currentState.Id()
  tcount := sid - id
  if tcount <= 0 || id < rs.trailingState.Id() {
    return sid, nil
  }

  transactions := make([]types.Transaction, tcount)
  elem := rs.transactionIdLookup[id]
  transactions[0] = elem.Value.(types.Transaction)
  for i := id + 1; i < sid; i++ {
    elem = elem.Next()
    transactions[i - id] = elem.Value.(types.Transaction)
  }
  return sid, transactions
}

/*
SubmitAndGetSince - composition of SubmitTransaction and TransactionsSinceId
*/
func (rs *Resolver) SubmitAndGetSince (t types.Transaction) (bool, []types.Transaction) {
  rs.mu.Lock()
  defer rs.mu.Unlock()

  if rs.closed {
    return false, nil
  }

  ok, _ := rs.submitTransactionLockless(t, true)
  _, xas := rs.transactionsSinceIdLockless(t.Id())
  return ok, xas
}

/*
CurrentStateId - Returns the id of the current state
*/
func (rs *Resolver) CurrentStateId() int64 {
  rs.mu.RLock()
  defer rs.mu.RUnlock()

  if rs.closed {
    return 0
  }

  return rs.currentState.Id()
}

/*
TrailingDistance - Returns the trailing distance of the resolver
*/
func (rs *Resolver) TrailingDistance() int {
  return rs.trailingDistance
}

/*
CurrentState - Calls the specified callback with the current state as its argument.
  Made to accept a callback so it can enforce proper concurrency control -- the readers' lock
  is held for the duration of the callback execution.

  The callback *MUST NOT* modify any field in the State!
*/
func (rs *Resolver) CurrentState(callback func(types.State)) {
  rs.mu.RLock()
  defer rs.mu.RUnlock()

  if rs.closed {
    return
  }

  callback(rs.currentState)
}

/*
submitTransactionLockless - lockless version of SubmitTransaction.
  Called by SubmitTransaction after acquiring the write lock on the Resolver. Then, appendToDurableLog = true
  Also called when restoring state from log. Then, appendToDurableLog = false

  Write lock on the Resolver must be held when calling this method.
*/
func (rs *Resolver) submitTransactionLockless(t types.Transaction, appendToDurableLog bool) (bool, types.Transaction) {
  trid := t.Id()
  sid := rs.currentState.Id()
  tsid := rs.trailingState.Id()

  /*
  Ensure at-most-once semantics: If the token provided by the transaction is the same as
  a transaction in the log, fail the transaction.
  */
  if oldT, exists := rs.requestTokens[t.GetToken()]; exists {
    debug.DPrintf(1, "Got duplicate request %v", t)
    return true, oldT.Value.(types.Transaction)
  }

  /*
  case 1: transaction is based on up-to-date state
    simply apply on top of the current state
    *must always succeed*
  */
  if trid == sid {
    ok, newT := rs.currentState.Apply(t)

    debug.Assert(ok,
                 "Current transaction %+v failed to apply against current state %+v",
                 t, rs.currentState)

    return rs.transactionSuccess(newT, appendToDurableLog)
  }

  /*
  case 2: transaction is based on state at or newer than trailing state
    attempt to apply on top of current state
    if that fails, attempt to resolve against log
  */
  if trid >= tsid && trid < sid {
    ok, newT := rs.currentState.Apply(t)

    if ok {
      return rs.transactionSuccess(newT, appendToDurableLog)
    }

    // failed to apply, attempt to resolve
    ok, newT = rs.currentState.Resolve(&rs.trailingState, rs.log, t)
    if ok {
      assertRecentTransaction(&rs.currentState, newT)

      ok, newT = rs.currentState.Apply(newT)

      debug.Assert(ok,
        "Failed to apply transaction %+v that was resolved successfully against state %+v",
        t, rs.currentState)

      return rs.transactionSuccess(newT, appendToDurableLog)
    }

    // failed to resolve, reject
    return false, nil
  }

  /*
  case 3: transaction is behind the trailing state,
    reject without trying to resolve.
    This helps preserve at-most-once semantics.
  */
  if trid < tsid {
    debug.DPrintf(1, "Rejecting %+v, too old. Currently at %d.", t, tsid)
    return false, nil
  }

  debug.Assert(false,
               "Unreachable case in SubmitTransaction: trid=%d tsid=%d sid=%d",
               trid, tsid, sid)
  return false, nil
}

/*
SubmitTransaction - Ask the resolver to apply a transaction.
  Returns success/failure, and the transaction as it looked when committed
  (committed transaction may be different than transaction passed in)
*/
func (rs *Resolver) SubmitTransaction(t types.Transaction) (bool, types.Transaction) {
  rs.mu.Lock()
  defer rs.mu.Unlock()

  if rs.closed {
    return false, nil
  }

  return rs.submitTransactionLockless(t, true)
}
