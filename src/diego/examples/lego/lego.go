package lego

// import "strconv"
import "container/list"
import "diego/debug"
import "diego/resolver"
import "fmt"
// import "diego/debug"

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
  size Vec2i
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
  size Vec2i
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
  data [][][]LegoBrick
}

func (universe *LegoUniverse) SetId(id int64) {
  universe.id = id
}

func (universe *LegoUniverse) Id() int64 {
  return universe.id
}

func (universe *LegoUniverse) Apply(t resolver.Transaction) (bool, resolver.Transaction) {
  xa := t.(*LegoTransaction)

  for _, op := range xa.ops {
    switch typedOp := op.(type) {
    case *LegoOpInsertBrick:
      fmt.Printf("Inserting brick at position = %d %d %d\n",
                 typedOp.position.data[0], typedOp.position.data[1], typedOp.position.data[2])
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
  result.data = make([][][]LegoBrick, LegoSize)
  for i := 0; i < LegoSize; i++ {
    result.data[i] = make([][]LegoBrick, LegoSize)
    for j := 0; j < LegoSize; j++ {
      result.data[i][j] = make([]LegoBrick, LegoSize)
    }
  }
  return result
// }
