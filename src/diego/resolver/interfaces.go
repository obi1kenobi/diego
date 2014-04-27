package resolver

import "container/list"

/*
Transaction - (set of) operations on a state
*/
type Transaction interface {
  Id() int64
}

/*
State - abstract notion of a state
*/
type State interface {
  SetId(id int64)
  Id() int64
  Apply(t Transaction) (bool, Transaction)
  Resolve(ancestorState *State, log *list.List, current Transaction) (bool, Transaction)
}
