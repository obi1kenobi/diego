package server

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
