package server

import "testing"
import "diego/tests"
import "diego/types"
import "diego/resolver"

const trailingDistance = 50

func setup()(*resolver.Resolver, *LegoUniverse) {
  return resolver.CreateResolver(MakeState, trailingDistance, ""), MakeState().(*LegoUniverse)
}

func TestLego(t *testing.T) {
  // rs, s := setup()
}

func makeTransaction(id int64, token types.RequestToken, ops []*LegoOp) *LegoTransaction {
  xa := &LegoTransaction{}
  xa.Tid = id
  xa.Token = token
  xa.Ops = ops
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
  gridId := universe.GetBrickIdAtPosition(expectedPosition)
  if brickId != gridId {
    t.Errorf("Brick id %d is not at position: %v", brickId, expectedPosition)
    return false
  }

  // Fetch brick from universe
  brick, _ := universe.GetBrick(brickId)

  // Check that brick matches expected brick id
  if brick.Bid != brickId {
    t.Errorf("Brick id %d is not at position: %v", brickId, expectedPosition)
    return false
  }

  // Check that position matches expected position
  if brick.Position != expectedPosition {
    t.Errorf("Brick id %d is not at correct position. Expected %v, got %v",
             brickId, expectedPosition, brick.Position)
    return false
  }

  // Check brick foot print in the universe
  if brick.Size != expectedSize {
    t.Errorf("Brick id %d is the wrong size. Expected %v, got %v",
             brickId, expectedSize, brick.Size)
    return false
  }
  start := []int32{ expectedPosition.data[0], expectedPosition.data[1], expectedPosition.data[2] }
  end := []int32{
    start[0] + brick.Size.data[0],
    start[1] + brick.Size.data[1],
    start[2] + brick.Size.data[2],
  }
  for x := start[0]; x < end[0]; x++ {
    for y := start[1]; y < end[1]; y++ {
      for z := start[2]; z < end[2]; z++ {
        gridBrickId := universe.readBrick(MakeVec3i(x, y, z))
        if brickId != gridBrickId {
          t.Errorf("Brick id %d was not inserted within region of its footprint. " +
                   "Unexpected brick id %d found at (%d, %d, %d)",
                   brickId, gridBrickId, x, y, z)
          return false
        }
      }
    }
  }

  // Check brick orientation
  if brick.Orientation != expectedOrientation {
    t.Errorf("Brick id %d has the wrong orientation. Expected %v, got %v",
             brickId, expectedOrientation, brick.Orientation)
    return false
  }

  // Check brick color
  if brick.Color != expectedColor {
    t.Errorf("Brick id %d has the wrong color. Expected %v, got %v",
             brickId, expectedColor, brick.Color)
    return false
  }

  return true
}

func legoCreatePredicate(ops []*LegoOp) func(*testing.T, types.State) bool {
  return func(t *testing.T, s types.State) bool {
    op := ops[0]
    universe := s.(*LegoUniverse)

    // Did something get inserted?
    for x := op.Position.data[0]; x < op.Position.data[0] + op.Size.data[0]; x++ {
      for y := op.Position.data[1]; y < op.Position.data[1] + op.Size.data[1]; y++ {
        for z := op.Position.data[2]; z < op.Position.data[2] + op.Size.data[2]; z++ {
          id := universe.GetBrickIdAtPosition(MakeVec3i(x, y, z))
            if id < 1 {
              t.Errorf("No brick was inserted at position %d %d %d\n",
                       op.Position.data[0], op.Position.data[1], op.Position.data[2])
                return false
            }
            // Does the brick match the one we are inserting?
            checkBrick(t, universe, id, op.Position, op.Size, op.Orientation, op.Color)
        }
      }
    }

    return true
  }
}

func stateEquals(a, b types.State) bool {
  return true
}

func TestCreateOp(t *testing.T) {
  rs, s := setup()

  // Transactions for tests and expected results
  xas := [][]*LegoOp {
    { &LegoOp { LegoOpCreateBrick, 0, MakeVec3i( 0, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) } },
    { &LegoOp { LegoOpCreateBrick, 0, MakeVec3i(10, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(0, 1, 0) } },
  }
  expectedResult := []tests.TransactionResult {
    tests.Success,
    tests.Success,
  }

  // Prepare tests
  testData := make([]tests.TestDataItem, len(xas))
  nt := types.MakeRequestTokenGenerator(0)
  for xaId, xa := range xas {
    testData[xaId] =
      tests.MakeTestDataItem(makeTransaction(int64(xaId), nt(), xa),
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
