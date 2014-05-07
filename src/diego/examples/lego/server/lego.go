package server

import "container/list"
import "diego/debug"
import "diego/resolver"
import "fmt"
import "reflect"

func (universe *LegoUniverse) Apply(t resolver.Transaction) (bool, resolver.Transaction) {
  xa := t.(*LegoTransaction)

  for _, op := range xa.ops {
    switch typedOp := op.(type) {
    case *LegoOpCreateBrick:
      universe.numBricks++
      brickId := universe.numBricks

      brick := LegoBrick {
        brickId, typedOp.position, typedOp.size, typedOp.orientation, typedOp.color,
      }
      universe.bricks[brickId] = brick
      universe.writeBrick(brickId, brick.position, brick.size)

      debug.DPrintf(1, "Inserted brick id %d at position = %v",
                    brickId, typedOp.position.data)
    case *LegoOpDeleteBrick:
      fmt.Printf("Deleting brick with id %d\n", typedOp.id)
    case *LegoOpModifyBrickColor:
      fmt.Printf("Modify brick id %d with color %f %f %f\n", typedOp.id,
                 typedOp.color.data[0], typedOp.color.data[1], typedOp.color.data[2])
    case *LegoOpModifyBrickOrientation:
      fmt.Printf("Modify brick id %d with orientation %s\n", typedOp.id,
                 typedOp.orientation)
    case *LegoOpModifyBrickSize:
      fmt.Printf("Modify brick id %d with size %d %d\n", typedOp.id,
                 typedOp.size.data[0], typedOp.size.data[1])
    default:
      debug.Assert(false, "Found invalid op: %v", op)
    }
  }

  return true, xa
}

func (universe *LegoUniverse) Resolve(ancestorState *resolver.State,
                                      log *list.List,
                                      current resolver.Transaction) (bool, resolver.Transaction) {
  // Ops:
  // - insert brick
  // - delete brick
  // - modify brick
  //
  // Conflict:
  //    - two users inserting the same brick at same position.
  //    - Resolution: instead of inserting a new brick, simply modify
  currentXa := current.(*LegoTransaction)
  currentOp := currentXa.ops[0]
  for e := log.Front(); e != nil; e = e.Next() {
    xa := e.Value.(*LegoTransaction)
    if xa.id < currentXa.id {
      continue
    }

    op := xa.ops[0]

    // Check for conflicting operations on same brick
    opId := getOpId(op)
    currentOpId := getOpId(currentOp)
    if opId != 0 && opId == currentOpId {
      // Check for modify conflicts
      if isModifyOp(currentOp) {
        // if currentOp.(type) == op.(type) {
        if reflect.TypeOf(currentOp) == reflect.TypeOf(op) {
          // Two conflicting modify ops
          return false, current
        }
        if isDeleteOp(op) {
          // Modify on a deleted brick
          return false, current
        }
      }
    }
  }
  return true, current
}

func MakeState() resolver.State {
  result := new(LegoUniverse)
  result.id = 0
  result.bricks = make(map[int64]LegoBrick)
  result.grid = make([][][]int64, LegoGridSize)
  for i := 0; i < LegoGridSize; i++ {
    result.grid[i] = make([][]int64, LegoGridSize)
    for j := 0; j < LegoGridSize; j++ {
      result.grid[i][j] = make([]int64, LegoGridSize)
    }
  }
  return result
}
