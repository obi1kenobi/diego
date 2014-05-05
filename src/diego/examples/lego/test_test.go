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

func legoInsertPredicate(xa LegoTransaction) func(*testing.T, resolver.State) bool {
  return func(t *testing.T, s resolver.State) bool {
    op := xa.ops[0].(LegoOpInsertBrick)
    universe := s.(*LegoUniverse)
    for x := xa.
  }
}

func stateEquals(a, b resolver.State) bool {
  return true
}

func TestInsertOp(t *testing.T) {
  rs, s := setup()

  xa0 := make([]interface{}, 0)

  xa0 = append(xa0, &LegoOpInsertBrick {
               MakeVec3i(0, 0, 0), MakeVec3i(2, 2, 1),
               BrickOrientationNorth, MakeVec3f(1, 0, 0) })

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(makeTransaction(0, xa0), tests.Success,
                           legoPredicate(ops0)),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}
