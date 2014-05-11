package server

import "testing"
import "diego/tests"
import "diego/types"
import "diego/resolver"

const trailingDistance = 50

func setup()(*resolver.Resolver, *LegoUniverse) {
  return resolver.CreateResolver(MakeState, trailingDistance, ""), MakeState().(*LegoUniverse)
}

func makeTransaction(id int64, ops []*LegoOp) *LegoTransaction {
  xa := &LegoTransaction{}
  xa.Tid = id
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

func legoCreatePredicate(expectedResult tests.TransactionResult,
                         ops []*LegoOp) func(*testing.T, types.State) bool {
  if expectedResult == tests.Success {
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
  } else {
    return nil
  }
}

func legoModifyPredicate(expectedResult tests.TransactionResult,
                         ops []*LegoOp) func(*testing.T, types.State) bool {
  if expectedResult == tests.Success {
    return func(t *testing.T, s types.State) bool {
      op := ops[0]
      universe := s.(*LegoUniverse)

      switch op.OpType {
      case LegoOpModifyBrickPosition:
        brick, _ := universe.GetBrick(op.BrickID)
        if brick.Position != op.Position ||
          !universe.validateBrickFootprint(op.BrickID, op.Position, brick.Size) {
          t.Errorf("Brick id %d did not move to desired position: %v",
                   op.BrickID, op.Position)
        }
      case LegoOpModifyBrickSize:
        brick, _ := universe.GetBrick(op.BrickID)
        if brick.Size != op.Size ||
          !universe.validateBrickFootprint(op.BrickID, brick.Position, op.Size) {
          t.Errorf("Brick id %d did not resize to desired footprint: %v",
                   op.BrickID, op.Size)
        }
      case LegoOpModifyBrickColor:
        brick, _ := universe.GetBrick(op.BrickID)
        if brick.Color != op.Color {
          t.Errorf("Brick id %d did not change to desired color %v", op.Color)
        }
      }

      return true
    }
  } else {
    return nil
  }
}

func stateEquals(a, b types.State) bool {
  return true
}

func TestCreateOp(t *testing.T) {
  rs, s := setup()

  // Transactions for tests and expected results
  xaIds := []int64 {
    0,
    0,
    1,
    1,
  }
  xas := [][]*LegoOp {
    { &LegoOp { LegoOpCreateBrick, 0, MakeVec3i( 0, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) } },
    { &LegoOp { LegoOpCreateBrick, 0, MakeVec3i( 0, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 1, 0) } },
    { &LegoOp { LegoOpCreateBrick, 0, MakeVec3i(10, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(0, 1, 0) } },
    { &LegoOp { LegoOpCreateBrick, 0, MakeVec3i( 9, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(0, 1, 0) } },
  }
  expectedResult := []tests.TransactionResult {
    tests.Success,
    tests.Failure,
    tests.Success,
    tests.Failure,
  }

  // Prepare tests
  testData := make([]tests.TestDataItem, len(xas))
  for index, xa := range xas {
    testData[index] =
      tests.MakeTestDataItem(makeTransaction(xaIds[index], xa),
                             expectedResult[index],
                             legoCreatePredicate(expectedResult[index], xas[index]))
  }

  // Run tests
  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

type SetupXa struct {
  id int64
  expected tests.TransactionResult
  ops []*LegoOp
}

func TestModifyOp(t *testing.T) {
  rs, s := setup()

  // Transactions for tests and expected results
  xas := []SetupXa {
    // Create brick #1 at (0, 0, 0)
    SetupXa {
      0,
      tests.Success,
      []*LegoOp { &LegoOp { LegoOpCreateBrick, 0, MakeVec3i( 0, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) } },
    },
    // Create brick #2 at (2, 0, 0)
    SetupXa {
      1,
      tests.Success,
      []*LegoOp { &LegoOp { LegoOpCreateBrick, 0, MakeVec3i( 2, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(0, 1, 0) } },
    },
    // Move brick #1 to (1, 0, 0) - conflict because brick #2 was created and there is now overlap
    SetupXa {
      1,
      tests.Failure,
      []*LegoOp { &LegoOp { OpType: LegoOpModifyBrickPosition, BrickID: 1, Position: MakeVec3i( 1, 0, 0) } },
    },
    // Create brick #3 at (0, 4, 0)
    SetupXa {
      2,
      tests.Success,
      []*LegoOp { &LegoOp { LegoOpCreateBrick, 0, MakeVec3i( 0, 4, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) } },
    },
    // Resize brick #1 to overlap brick #3 - conflict
    SetupXa {
      2,
      tests.Failure,
      []*LegoOp { &LegoOp { OpType: LegoOpModifyBrickSize, BrickID: 1, Size: MakeVec3i( 2, 6, 1) } },
    },
    // Resize brick #1 with no overlap to brick #3 - success
    SetupXa {
      2,
      tests.Success,
      []*LegoOp { &LegoOp { OpType: LegoOpModifyBrickSize, BrickID: 1, Size: MakeVec3i( 2, 4, 1) } },
    },
    // Change color of brick #1 and simultaneously move brick #1 - success
    SetupXa {
      4,
      tests.Success,
      []*LegoOp { &LegoOp { OpType: LegoOpModifyBrickColor, BrickID: 1, Color: MakeVec3f( 0, 0, 1) } },
    },
    SetupXa {
      4,
      tests.Success,
      []*LegoOp { &LegoOp { OpType: LegoOpModifyBrickPosition, BrickID: 1, Position: MakeVec3i(12, 0, 1) } },
    },
    // Change color of brick #2 and simultaneously both move brick #2 (ok) and change its color (conflict)
    SetupXa {
      6,
      tests.Success,
      []*LegoOp {
        &LegoOp { OpType: LegoOpModifyBrickColor, BrickID: 2, Color: MakeVec3f( 0, 0, 1) },
      },
    },
    SetupXa {
      6,
      tests.Failure,
      []*LegoOp {
        &LegoOp { OpType: LegoOpModifyBrickPosition, BrickID: 2, Position: MakeVec3i(12, 12, 1) },
        &LegoOp { OpType: LegoOpModifyBrickColor, BrickID: 2, Color: MakeVec3f( 1, 0, 1) },
      },
    },
  }

  // Prepare tests
  testData := make([]tests.TestDataItem, len(xas))
  for index, xa := range xas {
    testData[index] =
      tests.MakeTestDataItem(makeTransaction(xa.id, xa.ops),
                             xa.expected,
                             legoModifyPredicate(xa.expected, xa.ops))
  }

  // Run tests
  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}
