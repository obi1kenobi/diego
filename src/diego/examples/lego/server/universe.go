package server

import "diego/debug"
import "fmt"

var legoGridSize = []int32 { 64, 64, 64 }
var legoGridMin = []int32 { -31, -31, 0 }
var legoGridMax = []int32 { 32, 32, 63 }

type LegoUniverse struct {
  id int64
  bricks map[int64]*LegoBrick
  numBricks int64
  grid [][][]int64
}

func (universe *LegoUniverse) SetId(id int64) {
  universe.id = id
}

func (universe *LegoUniverse) Id() int64 {
  return universe.id
}

func validateBrickPosition(position Vec3i) {
  for i := 0; i < 3; i++ {
    debug.Assert(position.data[i] >= legoGridMin[i],
                 "Invalid grid position %d %d %d",
                 position.data[0], position.data[1], position.data[2])
    debug.Assert(position.data[i] <= legoGridMax[i],
                 "Invalid grid position %d %d %d",
                 position.data[0], position.data[1], position.data[2])
  }
}

func getGridIndex(position Vec3i) (int32, int32, int32) {
  x := position.data[0] - legoGridMin[0]
  y := position.data[1] - legoGridMin[1]
  z := position.data[2] - legoGridMin[2]
  return x, y, z
}

func (universe *LegoUniverse) insertBrick(brick *LegoBrick) {
  universe.bricks[brick.id] = brick
  universe.writeBrick(brick.id, brick.position, brick.size)
}

func (universe *LegoUniverse) moveBrick(brick *LegoBrick, newPosition Vec3i) {
  validateBrickPosition(newPosition)
  universe.writeBrick(0, brick.position, brick.size)
  brick.position = newPosition
  universe.writeBrick(brick.id, brick.position, brick.size)
}

func (universe *LegoUniverse) deleteBrick(brick *LegoBrick) {
  universe.writeBrick(0, brick.position, brick.size)
  delete(universe.bricks, brick.id)
  universe.numBricks--
}

func (universe *LegoUniverse) writeBrick(id int64, position Vec3i, size Vec3i) {
  validateBrickPosition(position)

  startX, startY, startZ := getGridIndex(position)
  endX := startX + size.data[0]
  endY := startY + size.data[1]
  endZ := startZ + size.data[2]
  for x := startX; x < endX; x++ {
    for y := startY; y < endY; y++ {
      for z := startZ; z < endZ; z++ {
        universe.grid[x][y][z] = id
      }
    }
  }
}

func (universe *LegoUniverse) readBrick(position Vec3i) int64 {
  validateBrickPosition(position)
  x, y, z := getGridIndex(position)
  return universe.grid[x][y][z]
}

func (universe *LegoUniverse) GetBrickIdAtPosition(position Vec3i) int64 {
  return universe.readBrick(position)
}

func (universe *LegoUniverse) GetBrick(id int64) (*LegoBrick, bool) {
  brick, ok := universe.bricks[id]
  return brick, ok
}

func (universe *LegoUniverse) displayLevel(level int) {
  for y := legoGridSize[1] - 1; y >= int32(0); y-- {
    for x := int32(0); x < legoGridSize[0]; x++ {
      fmt.Printf("%02d ", universe.grid[x][y][level])
    }
    fmt.Printf("\n")
  }
}
