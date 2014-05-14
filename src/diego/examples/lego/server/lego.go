package server

import "container/list"
import "diego/debug"
import "diego/types"
import "fmt"

func (universe *LegoUniverse) Apply(t types.Transaction) (bool, types.Transaction) {
  xa := t.(*LegoTransaction)

  if universe.id != xa.Tid {
    debug.DPrintf(1, "Xa (id=%d) is not against top of state (id=%d)\n", xa.Tid, universe.id)
    return false, nil
  }

  for _, op := range xa.Ops {
    if op.OpType == LegoOpCreateBrick {
      debug.Assert(universe.canWriteBrick(op.Position, op.Size),
                   "Creating a brick in area populated by another brick")

      universe.numBricks++
      universe.brickID++
      brickId := universe.brickID

      brick := &LegoBrick {
        brickId, op.Position, op.Size, op.Orientation, op.Color,
      }
      universe.insertBrick(brick)

      debug.DPrintf(1, "Inserted brick id %d at position = %v",
                    brickId, op.Position.data)
      fmt.Printf("Created brick id %d\n", brickId)
    } else {
      brick, ok := universe.bricks[op.BrickID]
      debug.Assert(ok, "BrickID %d not found\n", op.BrickID)

      switch op.OpType {
      case LegoOpDeleteBrick:
        debug.DPrintf(1, "Deleting brick with id %d\n", op.BrickID)
        fmt.Printf("Destroyed brick id %d\n", op.BrickID)
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
        universe.resizeBrick(brick, op.Size)
      default:
        debug.Assert(false, "Found invalid op: %v", op)
      }
    }
  }

  if false {
    universe.displayLevel(0)
  }

  return true, xa
}

func (universe *LegoUniverse) Resolve(ancestorState *types.State,
                                      log *list.List,
                                      untypedNewXa types.Transaction) (bool, types.Transaction) {
  // Ops:
  // - insert brick
  // - delete brick
  // - modify brick
  //
  // Conflicts:
  //
  // 1. Insert brick where brick already inserted.
  //    Resolution: Transaction rejected.
  //
  // 2. Modify brick that someone else modified.
  //    Resolution: Transaction accepted unless:
  //    a) Both ops modify the same aspect of the brick
  //       (position, color, orientation or size)
  //    b) Ops result in two bricks intersecting.
  //
  // 3. Modify brick that someone else deleted.
  //    Resolution: Transaction rejected.
  //
  // Note: The state machine (if/then/else) is not optimally set up; 
  // certain checks are done more than once but I intentionally opted 
  // for a more straighforward/readable version than something that 
  // would be hard to follow.
  //
  // XXX: We don't handle propertly the case where an op from the log
  // conflicts with an op from the new transaction but further down another
  // op comes in that removes the conflict (e.g., our transaction creates a
  // brick in a position where another op moves a brick into but yet
  // another op moves the brick out of the way).
  newXa := untypedNewXa.(*LegoTransaction)

  // Default resolution is to simply accept this transform and
  // bump the transaction id.
  resolvedXa := &LegoTransaction{}
  resolvedXa.Tid = universe.id
  resolvedXa.Ops = newXa.Ops

  for e := log.Front(); e != nil; e = e.Next() {
    xa := e.Value.(*LegoTransaction)

    // Ignore transactions you are already aware of
    if xa.Tid < newXa.Tid {
      continue
    }

    // Compare ops from both transactions and look for conflicts
    for _, op := range xa.Ops {
      for _, newOp := range newXa.Ops {
        if newOp.OpType == LegoOpCreateBrick {
          if op.OpType == LegoOpCreateBrick {
            brick := LegoBrick { Position: op.Position, Size: op.Size }
            newBrick := LegoBrick { Position: newOp.Position, Size: newOp.Size }
            if newBrick.Intersects(brick) {
              // Two newly created bricks occupy same space
              return false, nil
            }
          } else if op.isModifyOp() {
            brick, _ := universe.GetBrick(op.BrickID)
            updatedBrick := *brick
            updatedBrick.Apply(op)
            newOpBrick := LegoBrick { 0, op.Position, op.Size, op.Orientation, op.Color }
            if updatedBrick.Intersects(newOpBrick) {
              // Creating a brick where another brick moved into
              return false, nil
            }
          }
        } else if newOp.isModifyOp() {
          if op.OpType == LegoOpCreateBrick {
            brick := LegoBrick { 0, op.Position, op.Size, op.Orientation, op.Color }
            newOpBrick, _ := universe.GetBrick(newOp.BrickID)
            updatedNewOpBrick := *newOpBrick
            updatedNewOpBrick.Apply(newOp)
            if brick.Intersects(updatedNewOpBrick) {
              // Brick moves into area where a brick was created
              return false, nil
            }
          } else if newOp.BrickID == op.BrickID {
            if newOp.OpType == op.OpType && !newOp.equal(op) {
              // Both modify the same aspect of the same brick in a
              // different way.
              return false, nil
            }
            if op.OpType == LegoOpDeleteBrick {
              // Trying to modify a deleted brick
              return false, nil
            }
          } else if op.OpType != LegoOpDeleteBrick {
            brick, _ := universe.GetBrick(op.BrickID)
            updatedBrick := *brick
            updatedBrick.Apply(op)
            newOpBrick, _ := universe.GetBrick(op.BrickID)
            updatedNewOpBrick := *newOpBrick
            updatedNewOpBrick.Apply(newOp)
            if updatedBrick.Intersects(updatedNewOpBrick) {
              // Bricks intersect after each respective move/resize op
              return false, nil
            }
          }
        }
      }
    }
  }

  return true, resolvedXa
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
