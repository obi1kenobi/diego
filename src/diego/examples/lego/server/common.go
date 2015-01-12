package server

import "bytes"
import "strconv"
import "diego/debug"

type Vec2i struct {
  Data [2]int32
}

type Vec3f struct {
  Data [3]float32
}

type Vec3d struct {
  Data [3]float64
}

type Vec3u struct {
  Data [3]uint32
}

type Vec3i struct {
  Data [3]int32
}

func MakeVec2i(x, y int32) Vec2i {
  return Vec2i { [2]int32 { x, y } }
}

func MakeVec3i(x, y, z int32) Vec3i {
  return Vec3i { [3]int32 { x, y, z } }
}

func MakeVec3f(x, y, z float32) Vec3f {
  return Vec3f { [3]float32 { x, y, z } }
}

func MakeVec3d(x, y, z float64) Vec3d {
  return Vec3d { [3]float64 { x, y, z } }
}

func (vec *Vec2i) Add(other Vec2i) Vec2i {
  return MakeVec2i(vec.Data[0] + other.Data[0],
                   vec.Data[1] + other.Data[1])
}

func (vec *Vec3i) Add(other Vec3i) Vec3i {
  return MakeVec3i(vec.Data[0] + other.Data[0],
                   vec.Data[1] + other.Data[1],
                   vec.Data[2] + other.Data[2])
}

func (vec *Vec3f) Add(other Vec3f) Vec3f {
  return MakeVec3f(vec.Data[0] + other.Data[0],
                   vec.Data[1] + other.Data[1],
                   vec.Data[2] + other.Data[2])
}

func (vec *Vec3d) Add(other Vec3d) Vec3d {
  return MakeVec3d(vec.Data[0] + other.Data[0],
                   vec.Data[1] + other.Data[1],
                   vec.Data[2] + other.Data[2])
}

func (vec *Vec2i) Subtract(other Vec2i) Vec2i {
  return MakeVec2i(vec.Data[0] - other.Data[0],
                   vec.Data[1] - other.Data[1])
}

func (vec *Vec3i) Subtract(other Vec3i) Vec3i {
  return MakeVec3i(vec.Data[0] - other.Data[0],
                   vec.Data[1] - other.Data[1],
                   vec.Data[2] - other.Data[2])
}

func (vec *Vec3f) Subtract(other Vec3f) Vec3f {
  return MakeVec3f(vec.Data[0] - other.Data[0],
                   vec.Data[1] - other.Data[1],
                   vec.Data[2] - other.Data[2])
}

func (vec *Vec3d) Subtract(other Vec3d) Vec3d {
  return MakeVec3d(vec.Data[0] - other.Data[0],
                   vec.Data[1] - other.Data[1],
                   vec.Data[2] - other.Data[2])
}

func (vec *Vec3f) Multiply(constant float32) Vec3f {
  return MakeVec3f(vec.Data[0] * constant,
                   vec.Data[1] * constant,
                   vec.Data[2] * constant)
}

func (vec *Vec3d) Multiply(constant float64) Vec3d {
  return MakeVec3d(vec.Data[0] * constant,
                   vec.Data[1] * constant,
                   vec.Data[2] * constant)
}

func (vec *Vec3i) Divide(constant float64) Vec3d {
  return MakeVec3d(float64(vec.Data[0]) / constant,
                   float64(vec.Data[1]) / constant,
                   float64(vec.Data[2]) / constant)
}

func (vec *Vec3f) Divide(constant float32) Vec3f {
  return MakeVec3f(vec.Data[0] / constant,
                   vec.Data[1] / constant,
                   vec.Data[2] / constant)
}

func (vec *Vec3d) Divide(constant float64) Vec3d {
  return MakeVec3d(vec.Data[0] / constant,
                   vec.Data[1] / constant,
                   vec.Data[2] / constant)
}

func (vec *Vec2i) Equal(other Vec2i) bool {
  return vec.Data == other.Data
}

func (vec *Vec3i) Equal(other Vec3i) bool {
  return vec.Data == other.Data
}

func (vec *Vec3f) Equal(other Vec3f) bool {
  return vec.Data == other.Data
}

func serializeInt32(x int32, b *bytes.Buffer) {
  b.WriteByte(legoDelim)
  b.WriteString(strconv.FormatInt(int64(x), 10))
}

func deserializeInt32(b *bytes.Buffer) int32 {
  val, err := b.ReadString(legoDelim)
  debug.EnsureNoError(err)

  res, err := strconv.ParseInt(val[:len(val)-1], 10, 32)
  debug.EnsureNoError(err)

  return int32(res)
}

func serializeInt64(x int64, b *bytes.Buffer) {
  b.WriteByte(legoDelim)
  b.WriteString(strconv.FormatInt(x, 10))
}

func deserializeInt64(b *bytes.Buffer) int64 {
  val, err := b.ReadString(legoDelim)
  debug.EnsureNoError(err)

  res, err := strconv.ParseInt(val[:len(val)-1], 10, 64)
  debug.EnsureNoError(err)

  return res
}

func serializeUint64(x uint64, b *bytes.Buffer) {
  b.WriteByte(legoDelim)
  b.WriteString(strconv.FormatUint(x, 10))
}

func deserializeUint64(b *bytes.Buffer) uint64 {
  val, err := b.ReadString(legoDelim)
  debug.EnsureNoError(err)

  res, err := strconv.ParseUint(val[:len(val)-1], 10, 64)
  debug.EnsureNoError(err)

  return res
}

func serializeFloat32(x float32, b *bytes.Buffer) {
  b.WriteByte(legoDelim)
  b.WriteString(strconv.FormatFloat(float64(x), 'g', -1, 32))
}

func deserializeFloat32(b *bytes.Buffer) float32 {
  val, err := b.ReadString(legoDelim)
  debug.EnsureNoError(err)

  res, err := strconv.ParseFloat(val[:len(val)-1], 32)
  debug.EnsureNoError(err)

  return float32(res)
}

func (vec *Vec3i) serialize(b *bytes.Buffer) {
  for _, x := range vec.Data {
    serializeInt32(x, b)
  }
}

func (vec *Vec3i) deserialize(b *bytes.Buffer) {
  for i := range vec.Data {
    vec.Data[i] = deserializeInt32(b)
  }
}

func (vec *Vec3f) serialize(b *bytes.Buffer) {
  for _, x := range vec.Data {
    serializeFloat32(x, b)
  }
}

func (vec *Vec3f) deserialize(b *bytes.Buffer) {
  for i := range vec.Data {
    vec.Data[i] = deserializeFloat32(b)
  }
}
