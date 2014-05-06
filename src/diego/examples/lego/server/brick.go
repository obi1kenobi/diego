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
  id int64
  position Vec3i
  size Vec3i
  orientation BrickOrientation
  color Vec3f
}
