package resolver

import "container/list"
import "diego/debug"

/*
The default maximum distance between the current and the trailing state.
*/
const DefaultTrailingDistance = 1000

/*
Resolver - The main resolver object that this framework provides.
*/
type Resolver struct {
  currentState State
  trailingState State
  log *list.List // in order of oldest transaction (Front) to newest transaction (Back)
  trailingDistance int
}

/*
CreateResolver - Factory method to make a resolver based on a State-generating function.
*/
func CreateResolver(makeState func()State, trailingDistance int) *Resolver {
  rs := new(Resolver)
  rs.currentState = makeState()
  rs.trailingState = makeState()
  rs.currentState.SetId(0)
  rs.trailingState.SetId(0)
  rs.trailingDistance = trailingDistance
  rs.log = new(list.List)
  return rs
}

func (rs *Resolver) appendTransaction(t Transaction) {
  debug.Assert(rs.log.Len() <= rs.trailingDistance,
         "Log length %s > trailing distance %s",
         debug.Stringify(rs.log.Len()), debug.Stringify(rs.trailingDistance))

  if rs.log.Len() == rs.trailingDistance {
    sid := rs.trailingState.Id()
    oldT := rs.log.Front().Value.(Transaction)
    oldtrid := oldT.Id()

    debug.Assert(sid == oldtrid,
           "Id mismatch: sid %s != oldtrid %s",
           debug.Stringify(sid), debug.Stringify(oldtrid))

    // the log is full, first apply a transaction to the trailing state
    ok, _ := rs.trailingState.Apply(oldT)
    rs.trailingState.SetId(sid + 1)

    debug.Assert(ok,
                 "Failed to apply transaction %s to trailing state %s",
                 debug.Stringify(rs.log.Front().Value), debug.Stringify(rs.trailingState))

    rs.log.Remove(rs.log.Front())
  }

  rs.log.PushBack(t)
}

func assertRecentTransaction(s *State, t Transaction) {
  debug.Assert((*s).Id() == t.Id(),
               "Transaction %s not recent relative to state %s",
               debug.Stringify(t), debug.Stringify(s))
}

func (rs *Resolver) transactionSuccess(t Transaction) (bool, Transaction) {
  assertRecentTransaction(&rs.currentState, t)

  rs.currentState.SetId(rs.currentState.Id() + 1)
  rs.appendTransaction(t)
  return true, t
}

/*
SubmitTransaction - Ask the resolver to apply a transaction.
  Returns success/failure, and the transaction as it looked when committed
  (committed transaction may be different than transaction passed in)
*/
func (rs *Resolver) SubmitTransaction(t Transaction) (bool, Transaction) {
  trid := t.Id()
  sid := rs.currentState.Id()
  tsid := rs.trailingState.Id()

  /*
  case 1: transaction is based on up-to-date state
    simply apply on top of the current state
    *must always succeed*
  */
  if trid == sid {
    ok, newT := rs.currentState.Apply(t)

    debug.Assert(ok,
                 "Current transaction %s failed to apply against current state %s",
                 debug.Stringify(t), debug.Stringify(rs.currentState))

    return rs.transactionSuccess(newT)
  }

  /*
  case 2: transaction is based on state at or newer than trailing state
    attempt to apply on top of current state
    if that fails, attempt to resolve against log
  */
  if trid >= tsid && trid < sid {
    ok, newT := rs.currentState.Apply(t)

    if ok {
      return rs.transactionSuccess(newT)
    }

    // failed to apply, attempt to resolve
    ok, newT = rs.currentState.Resolve(&rs.trailingState, rs.log, t)
    if ok {
      assertRecentTransaction(&rs.currentState, newT)

      ok, newT = rs.currentState.Apply(newT)

      debug.Assert(ok,
        "Failed to apply transaction %s that was resolved successfully against state %s",
        debug.Stringify(t), debug.Stringify(rs.currentState))

      return rs.transactionSuccess(newT)
    }

    // failed to resolve, reject
    return false, nil
  }

  /*
  case 3: transaction is behind the trailing state
    attempt to apply to the current state
    if that fails, reject without trying to resolve
  */
  if trid < tsid {
    ok, newT := rs.currentState.Apply(t)

    if ok {
      return rs.transactionSuccess(newT)
    }

    // failed to apply, reject
    return false, nil
  }

  debug.Assert(false,
               "Unreachable case in SubmitTransaction: trid=%s tsid=%s sid=%s",
               debug.Stringify(trid), debug.Stringify(tsid), debug.Stringify(sid))
  return false, nil
}
