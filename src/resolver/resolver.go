package resolver

import "container/list"

/*
The maximum distance between the current and the trailing state.
*/
const trailingDistance = 1000

/*
Resolver - The main resolver object that this framework provides.
*/
type Resolver struct {
  currentState State
  trailingState State
  log *list.List
}

/*
CreateResolver - Factory method to make a resolver based on a State-generating function.
*/
func CreateResolver(makeState func()State) *Resolver {
  rs := new(Resolver)
  rs.currentState = makeState()
  rs.trailingState = makeState()
  rs.currentState.SetId(0)
  rs.trailingState.SetId(0)
  return rs
}

func (rs *Resolver) appendTransaction(t *Transaction) {
  Assert(rs.log.Len() <= trailingDistance,
         "Log length " + Stringify(rs.log.Len()) +
         "> trailing distance " + Stringify(trailingDistance))

  if rs.log.Len() == trailingDistance {
    sid := rs.trailingState.Id()
    oldT := rs.log.Back().Value.(*Transaction)
    oldtrid := (*oldT).Id()

    Assert(sid == oldtrid,
           "Id mismatch: sid " + Stringify(sid) +
           " != oldtrid " + Stringify(oldtrid))

    // the log is full, first apply a transaction to the trailing state
    ok, _ := rs.trailingState.Apply(oldT)
    rs.trailingState.SetId(sid + 1)

    Assert(ok,
           "Failed to apply transaction " + Stringify(rs.log.Back().Value) +
           " to trailing state " + Stringify(rs.trailingState))

    rs.log.Remove(rs.log.Back())
  }

  rs.log.PushFront(t)
}

/*
SubmitTransaction - Ask the resolver to apply a transaction.
  Returns success/failure, and the transaction as it looked when committed
  (committed transaction may be different than transaction passed in)
*/
func (rs *Resolver) SubmitTransaction(t *Transaction) (bool, *Transaction) {
  trid := (*t).Id()
  sid := rs.currentState.Id()
  tsid := rs.trailingState.Id()

  /*
  case 1: transaction is based on up-to-date state
    simply apply on top of the current state
    *must always succeed*
  */
  if trid == sid {
    ok, newT := rs.currentState.Apply(t)
    Assert(ok,
           "Current transaction " + Stringify(t) +
           " failed to apply against current state " + Stringify(rs.currentState))
    rs.currentState.SetId(sid + 1)
    rs.appendTransaction(newT)
    return ok, newT
  }

  /*
  case 2: transaction is based on state at or newer than trailing state
    attempt to apply on top of current state
    if that fails, attempt to resolve against log
  */
  if trid >= tsid && trid < sid {
    ok, newT := rs.currentState.Apply(t)

    if ok {
      rs.currentState.SetId(sid + 1)
      rs.appendTransaction(newT)
      return ok, newT
    }

    // failed to apply, attempt to resolve
    ok, newT = rs.currentState.Resolve(&rs.trailingState, rs.log, t)
    if ok {
      rs.currentState.SetId(sid + 1)
      rs.appendTransaction(newT)
      return ok, newT
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
      rs.currentState.SetId(sid + 1)
      rs.appendTransaction(newT)
      return ok, newT
    }

    // failed to apply, reject
    return false, nil
  }

  Assert(false,
         "Unreachable case in SubmitTransaction: trid=" + Stringify(trid) +
         " tsid=" + Stringify(tsid) +
         " sid=" + Stringify(sid))
  return false, nil
}
