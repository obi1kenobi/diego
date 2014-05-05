package lego

import "container/list"
import "diego/debug"
import "diego/resolver"
import "fmt"

// Lego bricks
const (
  BrickOrientationNorth = iota
  BrickOrientationEast
  BrickOrientationSouth
  BrickOrientationWest
)
type BrickOrientation int

type LegoBrick struct {
  id int64
  position Vec3i
  size Vec3i
  orientation BrickOrientation
  color Vec3f
}

// Ops
type LegoOpInsertBrick struct {
  position Vec3i
  size Vec3i
  orientation BrickOrientation
  color Vec3f
}

type LegoOpDeleteBrick struct {
  id int64
}

type LegoOpModifyBrickColor struct {
  id int64
  color Vec3f
}

type LegoOpModifyBrickOrientation struct {
  id int64
  orientation BrickOrientation
}

type LegoOpModifyBrickSize struct {
  id int64
  size Vec3i
}

// Transactions
type LegoTransaction struct {
  id int64
  ops []interface{}
}

func (xa *LegoTransaction) SetId(id int64) {
  xa.id = id
}

func (xa *LegoTransaction) Id() int64 {
  return xa.id
}

type LegoUniverse struct {
  id int64
  bricks map[int64]LegoBrick
  numBricks int64
  data [][][]int64
}

func (universe *LegoUniverse) SetId(id int64) {
  universe.id = id
}

func (universe *LegoUniverse) Id() int64 {
  return universe.id
}

func (universe *LegoUniverse) writeBrick(id int64, position, size Vec3i) {
  endX := position.data[0] + size.data[0]
  endY := position.data[1] + size.data[1]
  endZ := position.data[2] + size.data[2]
  for x := position.data[0]; x < endX; x++ {
    for y := position.data[1]; y < endY; y++ {
      for z := position.data[2]; z < endZ; z++ {
        universe.data[x][y][z] = id
      }
    }
  }
}

func (universe *LegoUniverse) Apply(t resolver.Transaction) (bool, resolver.Transaction) {
  xa := t.(*LegoTransaction)

  for _, op := range xa.ops {
    switch typedOp := op.(type) {
    case *LegoOpInsertBrick:
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
    return false, nil
}

func makeState() resolver.State {
  result := new(LegoUniverse)
  result.id = 0
  LegoSize := 100
  result.bricks = make(map[int64]LegoBrick)
  result.data = make([][][]int64, LegoSize)
  for i := 0; i < LegoSize; i++ {
    result.data[i] = make([][]int64, LegoSize)
    for j := 0; j < LegoSize; j++ {
      result.data[i][j] = make([]int64, LegoSize)
    }
  }
  return result
}
