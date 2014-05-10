package helpers

import "container/list"

import "diego/debug"
import "diego/types"

func walkLogForward(log *list.List, count int64)*list.Element {
  e := log.Front()
  for i := int64(1); i < count; i++ {
    e = e.Next()
  }
  return e
}

func walkLogBackward(log *list.List, count int64)*list.Element {
  e := log.Back()
  for i := int64(1); i < count; i++ {
    e = e.Prev()
  }
  return e
}

/*
FindTransactionById - scans the transaction log for the node that contains
  the transaction with the specified id

  Assumes the log is composed of sequentially increasing entries
  (add to back, pop from front).
*/
func FindTransactionById(log *list.List, id int64)*list.Element {
  front := log.Front().Value.(types.Transaction).Id()
  back := log.Back().Value.(types.Transaction).Id()

  if front > id || back < id {
    // the transaction id is not in the log
    return nil
  }

  frontDiff := id - front
  backDiff := back - id
  debug.Assert(frontDiff >= 0 && backDiff >= 0, "Diffs are bad: %d %d", frontDiff, backDiff)

  if frontDiff <= backDiff {
    return walkLogForward(log, frontDiff)
  }

  return walkLogBackward(log, backDiff)
}

/*
CreateManagedResolver - makes a managed resolver based on the knowledge of which transactions commute with each other,
                      and which transactions resolve against each other.
*/
func CreateManagedResolver(makeContext func(current types.Transaction, ancestorState *types.State)interface{},
                           updateContext func(current, existing types.Transaction, context interface{}),
                           commutesWith func(current, existing types.Transaction, context interface{})bool,
                           resolvesWith func(current,
                                             existing types.Transaction,
                                             context interface{})(bool, types.Transaction))func(ancestorState *types.State,
                                                                                                log *list.List,
                                                                                                current types.Transaction)(bool, types.Transaction) {
  // oh don't I wish Go had proper multi-line statements
  return func (ancestorState *types.State, log *list.List,
               current types.Transaction) (bool, types.Transaction) {
    context := makeContext(current, ancestorState)
    var e *list.Element

    if context == nil {
      // no-context op
      // can use shortest path through log
      e = FindTransactionById(log, current.Id())
    } else {
      e = log.Front()
      diff := current.Id() - e.Value.(types.Transaction).Id()
      for i := int64(1); i < diff; i++ {
        updateContext(current, e.Value.(types.Transaction), context)
        e = e.Next()
      }
    }

    newT := current
    var success bool
    var existing types.Transaction

    for ; e != nil; e = e.Next() {
      existing = e.Value.(types.Transaction)
      if !commutesWith(newT, existing, context) {
        success, newT = resolvesWith(newT, existing, context)
        if !success {
          return false, nil
        }
      }
    }
    newT.SetId(log.Back().Value.(types.Transaction).Id() + 1)
    return true, newT
  }
}
