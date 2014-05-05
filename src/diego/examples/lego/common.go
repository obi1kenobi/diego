package lego

type Vec2i struct {
  data [2]int32
}

type Vec3f struct {
  data [3]float32
}

type Vec3d struct {
  data [3]float64
}

type Vec3u struct {
  data [3]uint32
}

type Vec3i struct {
  data [3]int32
}

func MakeVec3i(x, y, z int32) Vec3i {
  return Vec3i { [3]int32 { x, y, z } }
}

func MakeVec3f(x, y, z float32) Vec3f {
  return Vec3f { [3]float32 { x, y, z } }
}
