package types

import "container/list"

/*
Transaction - (set of) operations on a state
*/
type Transaction interface {
  Id() int64
  SetId(id int64)
  GetToken() RequestToken
}

/*
State - abstract notion of a state
*/
type State interface {
  Id() int64
  SetId(id int64)
  Apply(t Transaction) (bool, Transaction)
  Resolve(ancestorState *State, log *list.List, current Transaction) (bool, Transaction)
}

/*
RequestToken - for ensuring at most once of transactions.
Should be immutable.
*/
type RequestToken struct {
  ClientId int64
  ReqId int64
}
