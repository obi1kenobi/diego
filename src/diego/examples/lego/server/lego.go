package server

import "container/list"
import "diego/debug"
import "diego/types"
import "reflect"

func (universe *LegoUniverse) Apply(t types.Transaction) (bool, types.Transaction) {
  xa := t.(*LegoTransaction)

  // XXX: We blindly apply all transactions here, regardless of current state

  for _, op := range xa.Ops {
    if op.OpType == LegoOpCreateBrick {
      universe.numBricks++
      brickId := universe.numBricks

      brick := &LegoBrick {
        brickId, op.Position, op.Size, op.Orientation, op.Color,
      }
      universe.insertBrick(brick)

      debug.DPrintf(1, "Inserted brick id %d at position = %v",
                    brickId, op.Position.data)
    } else {
      brick, ok := universe.bricks[op.BrickID]
      if !ok {
        debug.DPrintf(1, "BrickID %d not found\n", op.BrickID)
        return false, nil
      }
      switch op.OpType {
      case LegoOpDeleteBrick:
        debug.DPrintf(1, "Deleting brick with id %d\n", op.BrickID)
        universe.deleteBrick(brick)
      case LegoOpModifyBrickPosition:
        debug.DPrintf(1, "Modify brick id %d position to %d %d %d\n", op.BrickID,
                      op.Position.data[0], op.Position.data[1], op.Position.data[2])
        universe.moveBrick(brick, op.Position)
      case LegoOpModifyBrickColor:
        debug.DPrintf(1, "Modify brick id %d color to %f %f %f\n", op.BrickID,
                      op.Color.data[0], op.Color.data[1], op.Color.data[2])
        brick.Color = op.Color
      case LegoOpModifyBrickOrientation:
        debug.DPrintf(1, "Modify brick id %d orientation to %s\n", op.BrickID,
                      op.Orientation)
        brick.Orientation = op.Orientation
      case LegoOpModifyBrickSize:
        debug.DPrintf(1, "Modify brick id %d size to %d %d %d\n", op.BrickID,
                      op.Size.data[0], op.Size.data[1], op.Size.data[2])
        brick.Size = op.Size
      default:
        debug.Assert(false, "Found invalid op: %v", op)
      }
    }
  }

  if (false) {
    universe.displayLevel(0)
  }

  return true, xa
}

func (universe *LegoUniverse) Resolve(ancestorState *types.State,
                                      log *list.List,
                                      current types.Transaction) (bool, types.Transaction) {
  // Ops:
  // - insert brick
  // - delete brick
  // - modify brick
  //
  // Conflict:
  //    - two users inserting the same brick at same position.
  //    - Resolution: instead of inserting a new brick, simply modify
  currentXa := current.(*LegoTransaction)
  currentOp := currentXa.Ops[0]
  for e := log.Front(); e != nil; e = e.Next() {
    xa := e.Value.(*LegoTransaction)
    if xa.Tid < currentXa.Tid {
      continue
    }

    op := xa.Ops[0]

    // Check for conflicting operations on same brick
    opId := op.BrickID
    currentOpId := currentOp.BrickID
    if opId != 0 && opId == currentOpId {
      // Check for modify conflicts
      if currentOp.isModifyOp() {
        // if currentOp.(type) == op.(type) {
        if reflect.TypeOf(currentOp) == reflect.TypeOf(op) {
          // Two conflicting modify ops
          return false, current
        }
        if op.OpType == LegoOpDeleteBrick {
          // Modify on a deleted brick
          return false, current
        }
      }
    }
  }
  return true, current
}

func MakeState() types.State {
  result := new(LegoUniverse)
  result.id = 0
  result.bricks = make(map[int64]*LegoBrick)
  result.grid = make([][][]int64, legoGridSize[0])
  for i := int32(0); i < legoGridSize[0]; i++ {
    result.grid[i] = make([][]int64, legoGridSize[1])
    for j := int32(0); j < legoGridSize[1]; j++ {
      result.grid[i][j] = make([]int64, legoGridSize[2])
    }
  }
  return result
}
