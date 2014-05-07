package server

import "bytes"

const createBrick = "CreateBrick"
const deleteBrick = "DeleteBrick"
const modifyBrickColor = "BrickColor"
const modifyBrickOrientation = "BrickOrientation"
const modifyBrickSize = "BrickSize"

// Ops
type LegoOpCreateBrick struct {
  position Vec3i
  size Vec3i
  orientation BrickOrientation
  color Vec3f
}

type LegoOpDeleteBrick struct {
  id int64
}

type LegoOpModifyBrickColor struct {
  id int64
  color Vec3f
}

type LegoOpModifyBrickOrientation struct {
  id int64
  orientation BrickOrientation
}

type LegoOpModifyBrickSize struct {
  id int64
  size Vec3i
}

func getOpId(op interface{}) int64 {
  switch typedOp := op.(type) {
  case *LegoOpCreateBrick: return 0
  case *LegoOpModifyBrickColor: return typedOp.id
  case *LegoOpModifyBrickOrientation: return typedOp.id
  case *LegoOpModifyBrickSize: return typedOp.id
  case *LegoOpDeleteBrick: return typedOp.id
  }
  return 0
}

func isModifyOp(op interface{}) bool {
  switch op.(type) {
  case *LegoOpModifyBrickColor: return true
  case *LegoOpModifyBrickOrientation: return true
  case *LegoOpModifyBrickSize: return true
  }
  return false
}

func isDeleteOp(op interface{}) bool {
  _, ok := op.(*LegoOpDeleteBrick)
  return ok
}

func (op *LegoOpCreateBrick) serialize(b *bytes.Buffer) {
  b.WriteString(createBrick)
  op.position.serialize(b)
  op.size.serialize(b)
  serializeInt32(int32(op.orientation), b)
  op.color.serialize(b)
  b.WriteByte('\n')
}

func (op *LegoOpCreateBrick) deserialize(b *bytes.Buffer) {
  op.position.deserialize(b)
  op.size.deserialize(b)
  op.orientation = BrickOrientation(deserializeInt32(b))
  op.color.deserialize(b)
}
