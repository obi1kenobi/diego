package server

import "bytes"
import "strconv"
import "diego/debug"

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

func (vec *Vec2i) Equal(other Vec2i) bool {
  return vec.data == other.data
}

func (vec *Vec3i) Equal(other Vec3i) bool {
  return vec.data == other.data
}

func (vec *Vec3f) Equal(other Vec3f) bool {
  return vec.data == other.data
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
  for _, x := range vec.data {
    serializeInt32(x, b)
  }
}

func (vec *Vec3i) deserialize(b *bytes.Buffer) {
  for i := range vec.data {
    vec.data[i] = deserializeInt32(b)
  }
}

func (vec *Vec3f) serialize(b *bytes.Buffer) {
  for _, x := range vec.data {
    serializeFloat32(x, b)
  }
}

func (vec *Vec3f) deserialize(b *bytes.Buffer) {
  for i := range vec.data {
    vec.data[i] = deserializeFloat32(b)
  }
}
