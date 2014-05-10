package helpers

import "diego/types"

/*
ApplyIfUpToDate - calls the applier only if the transaction is up-to-date with the state.
*/
func ApplyIfUpToDate(s types.State, t types.Transaction,
                     applier func(types.State, types.Transaction)types.Transaction) (bool, types.Transaction) {
  if s.Id() == t.Id() {
    return true, applier(s, t)
  }
  return false, nil
}

/*
ApplyIfSafe - calls isSafe to determine if the transaction is safe to apply,
              and if so, calls the applier; otherwise, refuses to apply.
*/
func ApplyIfSafe(s types.State, t types.Transaction,
                 isSafe func(types.State, types.Transaction)bool,
                 applier func(types.State, types.Transaction)types.Transaction) (bool, types.Transaction) {
  if isSafe(s, t) {
    return true, applier(s, t)
  }
  return false, nil
}
