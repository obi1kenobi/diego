package lego

const LegoGridSize = 100

type LegoUniverse struct {
  id int64
  bricks map[int64]LegoBrick
  numBricks int64
  grid [][][]int64
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
        universe.grid[x][y][z] = id
      }
    }
  }
}

func (universe *LegoUniverse) GetBrickIdAtPosition(position Vec3i) (int64, bool) {
  for i := 0; i < 3; i++ {
    if position.data[i] < 0 || position.data[i] >= LegoGridSize {
      return 0, false
    }
  }
  xyz := position.data
  return universe.grid[xyz[0]][xyz[1]][xyz[2]], true
}

func (universe *LegoUniverse) GetBrick(id int64) (LegoBrick, bool) {
  brick, ok := universe.bricks[id]
  return brick, ok
}
