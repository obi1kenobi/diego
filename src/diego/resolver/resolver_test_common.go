package resolver

import "diego/debug"
import "diego/types"

/*
AssertResolversEqual - see if two resolvers are semantically equal
*/
func AssertResolversEqual (r1, r2 *Resolver) {
  debug.Assert(r1 != nil, "resolver 1 did not exist")
  debug.Assert(r2 != nil, "resolver 2 did not exist")

  r1.mu.Lock()
  defer r1.mu.Unlock()
  r2.mu.Lock()
  defer r2.mu.Unlock()

  debug.Assert(r1.currentState == r2.currentState,
      "resolver current states %v and %v not equal", r1.currentState, r2.currentState)
  debug.Assert(r1.trailingState == r2.trailingState,
      "resolver trailing states %v and %v not equal", r2.trailingState, r2.trailingState)
  e1 := r1.log.Front()
  e2 := r2.log.Front()
  for e1 != nil && e2 != nil {
    debug.Assert(e1.Value.(types.Transaction) == e2.Value.(types.Transaction),
      "transactions %v and %v are not equal", e1.Value, e2.Value)
  }
  debug.Assert(e1 == nil, "length mismatch... transaction %v should not be in log", e1.Value)
  debug.Assert(e2 == nil, "length mismatch... transaction %v should not be in log", e2.Value)
}
