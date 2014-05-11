package server

import "math"

// Lego bricks
const (
  BrickOrientationNorth = iota
  BrickOrientationEast
  BrickOrientationSouth
  BrickOrientationWest
)
type BrickOrientation int

type LegoBrick struct {
  Bid int64
  Position Vec3i
  Size Vec3i
  Orientation BrickOrientation
  Color Vec3f
}

func (brick *LegoBrick) Center() Vec3d {
  var result Vec3d
  for i := 0; i < 3; i++ {
    result.data[i] = float64(brick.Position.data[i]) + float64(brick.Size.data[i]) / 2.0
  }
  return result
}

func (brick *LegoBrick) Intersects(other LegoBrick) bool {
  center := brick.Center()
  otherCenter := other.Center()
  t := otherCenter.Subtract(center)
  halfSize := brick.Size.Divide(float64(2.0))
  otherHalfSize := other.Size.Divide(float64(2.0))
  return math.Abs(t.data[0]) < (halfSize.data[0] + otherHalfSize.data[0]) &&
         math.Abs(t.data[1]) < (halfSize.data[1] + otherHalfSize.data[1]) &&
         math.Abs(t.data[2]) < (halfSize.data[2] + otherHalfSize.data[2])
}

func (brick *LegoBrick) Apply(op *LegoOp) {
  switch op.OpType {
  case LegoOpModifyBrickPosition:
    brick.Position = op.Position
  case LegoOpModifyBrickColor:
    brick.Color = op.Color
  case LegoOpModifyBrickOrientation:
    brick.Orientation = op.Orientation
  case LegoOpModifyBrickSize:
    brick.Size = op.Size
  }
}
