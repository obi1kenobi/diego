package server

import "testing"
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

func checkBrickId(t *testing.T, universe *LegoUniverse, id int64) bool {
  _, ok := universe.GetBrick(id)
  if !ok {
    t.Errorf("Brick id %d is unknown to the universe.", id)
    return false
  }
  return true
}

func checkBrick(t *testing.T,
                universe *LegoUniverse,
                brickId int64,
                expectedPosition Vec3i,
                expectedSize Vec3i,
                expectedOrientation BrickOrientation,
                expectedColor Vec3f) bool {
  // Check for valid brick ID
  if !checkBrickId(t, universe, brickId) {
    return false
  }

  // Check that brick was inserted at given position
  gridId, ok := universe.GetBrickIdAtPosition(expectedPosition)
  if !ok || brickId != gridId {
    t.Errorf("Brick id %d is not at position: %v", brickId, expectedPosition)
    return false
  }

  // Fetch brick from universe
  brick, _ := universe.GetBrick(brickId)

  // Check that brick matches expected brick id
  if brick.id != brickId {
    t.Errorf("Brick id %d is not at position: %v", brickId, expectedPosition)
    return false
  }

  // Check that position matches expected position
  if brick.position != expectedPosition {
    t.Errorf("Brick id %d is not at correct position. Expected %v, got %v",
             brickId, expectedPosition, brick.position)
    return false
  }

  // Check brick foot print in the universe
  if brick.size != expectedSize {
    t.Errorf("Brick id %d is the wrong size. Expected %v, got %v",
             brickId, expectedSize, brick.size)
    return false
  }
  start := []int32{ expectedPosition.data[0], expectedPosition.data[1], expectedPosition.data[2] }
  end := []int32{
    start[0] + brick.size.data[0],
    start[1] + brick.size.data[1],
    start[2] + brick.size.data[2],
  }
  for x := start[0]; x < end[0]; x++ {
    for y := start[1]; y < end[1]; y++ {
      for z := start[2]; z < end[2]; z++ {
        if brickId != universe.grid[x][y][z] {
          t.Errorf("Brick id %d was not inserted within region of its footprint", brickId)
          return false
        }
      }
    }
  }

  // Check brick orientation
  if brick.orientation != expectedOrientation {
    t.Errorf("Brick id %d has the wrong orientation. Expected %v, got %v",
             brickId, expectedOrientation, brick.orientation)
    return false
  }

  // Check brick color
  if brick.color != expectedColor {
    t.Errorf("Brick id %d has the wrong color. Expected %v, got %v",
             brickId, expectedColor, brick.color)
    return false
  }

  return true
}

func legoCreatePredicate(ops []interface{}) func(*testing.T, resolver.State) bool {
  return func(t *testing.T, s resolver.State) bool {
    op := ops[0].(*LegoOpCreateBrick)
    universe := s.(*LegoUniverse)

    // Did something get inserted?
    id, ok := universe.GetBrickIdAtPosition(op.position)
    if !ok || id < 1 {
      t.Errorf("No brick was inserted at position %d %d %d\n",
               op.position.data[0], op.position.data[1], op.position.data[2])
      return false
    }

    // Does the brick match the one we are inserting?
    checkBrick(t, universe, id, op.position, op.size, op.orientation, op.color)

    return true
  }
}

func stateEquals(a, b resolver.State) bool {
  return true
}

func TestCreateOp(t *testing.T) {
  rs, s := setup()

  // Transactions for tests and expected results
  xas := [][]interface{} {
    { &LegoOpCreateBrick { MakeVec3i(0, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) } },
    { &LegoOpCreateBrick { MakeVec3i(10, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) } },
  }
  expectedResult := []tests.TransactionResult {
    tests.Success,
    tests.Success,
  }

  // Prepare tests
  testData := make([]tests.TestDataItem, len(xas))
  for xaId, xa := range xas {
    testData[xaId] =
      tests.MakeTestDataItem(makeTransaction(int64(xaId), xa),
                             expectedResult[xaId],
                             legoCreatePredicate(xa))
  }

  // Run tests
  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

/*
func TestModifyOp(t *testing.T) {
  rs, s := setup()

  universe := s.(*LegoUniverse)

  // Transactions for tests and expected results
  xas := [][]interface{} {
    { &LegoOpCreateBrick { MakeVec3i(0, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) } },
    { &LegoOpModifyBrickSize { MakeVec3i(10, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) } },
  }
  expectedResult := []tests.TransactionResult {
    tests.Success,
    tests.Success,
  }

  // Prepare tests
  testData := make([]tests.TestDataItem, len(xas))
  for xaId, xa := range xas {
    testData[xaId] =
      tests.MakeTestDataItem(makeTransaction(int64(xaId), xa),
                             expectedResult[xaId],
                             legoCreatePredicate(xa))
  }

  // Run tests
  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}
*/
