package server

import "bytes"
import "diego/debug"

const (
  LegoOpCreateBrick = "CreateBrick"
  LegoOpDeleteBrick = "DeleteBrick"
  LegoOpModifyBrickPosition = "ModifyBrickPosition"
  LegoOpModifyBrickColor = "ModifyBrickColor"
  LegoOpModifyBrickOrientation = "ModifyBrickOrientation"
  LegoOpModifyBrickSize = "ModifyBrickSize"
)
type LegoOpType string

// Ops
type LegoOp struct {
  OpType LegoOpType
  BrickID int64
  Position Vec3i
  Size Vec3i
  Orientation BrickOrientation
  Color Vec3f
}

func (op *LegoOp) equal(other *LegoOp) bool {
  if op.OpType != other.OpType {
    // Different op type
    return false
  } else if op.OpType == LegoOpCreateBrick {
    // If create, compare brick characteristics
    return op.Position == other.Position &&
      op.Size == other.Size &&
      op.Orientation == other.Orientation &&
      op.Color == other.Color
  } else {
    // All other ops are on a given brick ID, so, first check for a
    // mismatch there.
    if op.BrickID != other.BrickID {
      return false
    } else {
      // Now check for remaining modify differences
      switch op.OpType {
      case LegoOpModifyBrickPosition:
        return op.Position == other.Position
      case LegoOpModifyBrickColor:
        return op.Color == other.Color
      case LegoOpModifyBrickOrientation:
          return op.Orientation == other.Orientation
      case LegoOpModifyBrickSize:
          return op.Size == other.Size
      default:
          // This should never be reached.
          return false
      }
    }
  }
}

func (op *LegoOp) isModifyOp() bool {
  switch op.OpType {
  case LegoOpModifyBrickPosition: return true
  case LegoOpModifyBrickSize: return true
  case LegoOpModifyBrickOrientation: return true
  case LegoOpModifyBrickColor: return true
  }
  return false
}

func (op *LegoOp) isModifyFootprintOp() bool {
  return op.OpType == LegoOpModifyBrickPosition ||
         op.OpType == LegoOpModifyBrickSize
}

func (op *LegoOp) serialize(b *bytes.Buffer) {
  switch op.OpType {
  case LegoOpCreateBrick:
    b.WriteString(LegoOpCreateBrick)
    op.Position.serialize(b)
    op.Size.serialize(b)
    serializeInt32(int32(op.Orientation), b)
    op.Color.serialize(b)
  case LegoOpDeleteBrick:
    b.WriteString(LegoOpDeleteBrick)
    serializeInt64(op.BrickID, b)
  case LegoOpModifyBrickPosition:
    b.WriteString(LegoOpModifyBrickPosition)
    serializeInt64(op.BrickID, b)
    op.Position.serialize(b)
  case LegoOpModifyBrickSize:
    b.WriteString(LegoOpModifyBrickSize)
    serializeInt64(op.BrickID, b)
    op.Size.serialize(b)
  case LegoOpModifyBrickColor:
    b.WriteString(LegoOpModifyBrickColor)
    serializeInt64(op.BrickID, b)
    op.Color.serialize(b)
  case LegoOpModifyBrickOrientation:
    b.WriteString(LegoOpModifyBrickOrientation)
    serializeInt64(op.BrickID, b)
    serializeInt32(int32(op.Orientation), b)
  default:
    debug.Assert(false, "Unknown op type: %v", op.OpType)
  }
  b.WriteByte('\n')
}

func (op *LegoOp) deserialize(b *bytes.Buffer) {
  opType, err := b.ReadString(legoDelim)
  debug.EnsureNoError(err)
  opType = opType[:len(opType) - 1]
  switch opType {
  case LegoOpCreateBrick:
    op.OpType = LegoOpCreateBrick
    op.Position.deserialize(b)
    op.Size.deserialize(b)
    op.Orientation = BrickOrientation(deserializeInt32(b))
    op.Color.deserialize(b)
  case LegoOpDeleteBrick:
    op.OpType = LegoOpDeleteBrick
    op.BrickID = deserializeInt64(b)
  case LegoOpModifyBrickPosition:
    op.OpType = LegoOpModifyBrickPosition
    op.BrickID = deserializeInt64(b)
    op.Position.deserialize(b)
  case LegoOpModifyBrickSize:
    op.OpType = LegoOpCreateBrick
    op.BrickID = deserializeInt64(b)
    op.Size.deserialize(b)
  case LegoOpModifyBrickColor:
    op.OpType = LegoOpModifyBrickColor
    op.BrickID = deserializeInt64(b)
    op.Color.deserialize(b)
  case LegoOpModifyBrickOrientation:
    op.OpType = LegoOpModifyBrickOrientation
    op.BrickID = deserializeInt64(b)
    op.Orientation = BrickOrientation(deserializeInt32(b))
  default:
    debug.Assert(false, "Unknown op type: %v", op.OpType)
  }
}
