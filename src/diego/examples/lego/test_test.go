package lego

import "testing"
// import "strings"
// import "strconv"
// import "diego/debug"
import "diego/tests"
import "diego/resolver"

const trailingDistance = 50

func setup()(*resolver.Resolver, *LegoUniverse) {
  return resolver.CreateResolver(makeState, trailingDistance), makeState().(*LegoUniverse)
}

func TestLego(t *testing.T) {
  // rs, s := setup()
}

func makeTransaction(id int64, ops []interface{}) *LegoTransaction {
  xa := &LegoTransaction{}
  xa.id = id
  xa.ops = ops
  return xa
}

func legoInsertPredicate(ops []interface{}) func(*testing.T, resolver.State) bool {
  return func(t *testing.T, s resolver.State) bool {
    op := ops[0].(*LegoOpInsertBrick)
    universe := s.(*LegoUniverse)
    id := universe.data[op.position.data[0]][op.position.data[1]][op.position.data[2]]
    if id < 1 {
      t.Errorf("Brick was not inserted at position %d %d %d\n",
               op.position.data[0], op.position.data[1], op.position.data[2])
      return false
    }
    for x := int32(0); x < op.size.data[0]; x++ {
      for y := int32(0); y < op.size.data[1]; y++ {
        for z := int32(0); z < op.size.data[2]; z++ {
          if id != universe.data[x][y][z] {
            t.Errorf("Brick was not inserted within region of footprint")
            return false
          }
        }
      }
    }

    brick := universe.bricks[id]
    if !brick.size.Equal(op.size) || !brick.color.Equal(op.color) || brick.orientation != op.orientation {
      return false
    }

    return true
  }
}

func stateEquals(a, b resolver.State) bool {
  return true
}

func TestInsertOp(t *testing.T) {
  rs, s := setup()

  ops := make([]interface{}, 0)

  ops = append(ops, &LegoOpInsertBrick {
               MakeVec3i(0, 0, 0), MakeVec3i(2, 2, 1),
               BrickOrientationNorth, MakeVec3f(1, 0, 0) })

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(makeTransaction(0, ops), tests.Success, legoInsertPredicate(ops)),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}
