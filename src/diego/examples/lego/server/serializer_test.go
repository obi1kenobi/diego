package server

import "bytes"
import "testing"

func TestSerializeCreateOp(t *testing.T) {
  op := &LegoOpCreateBrick { MakeVec3i(0, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) }
  ops := []interface{} { op }
  xa := &LegoTransaction{0, ops}
  ns := "testNs"

  b := new(bytes.Buffer)
  serializeTransaction(ns, xa, b)

  resNs, resXa := deserializeTransaction(b)

  if ns != resNs {
    t.Errorf("expected ns=%s got resNs=%s", ns, resNs)
  }

  if len(xa.ops) != len(resXa.ops) {
    t.Errorf("expected len(xa.ops)=%d got len(resXa.ops)=%d", len(xa.ops), len(resXa.ops))
  }

  resOp := resXa.ops[0].(*LegoOpCreateBrick)
  if !op.position.Equal(resOp.position) {
    t.Errorf("positions not equal: %v vs %v", op, resOp)
  }
  if !op.size.Equal(resOp.size) {
    t.Errorf("sizes not equal: %v vs %v", op, resOp)
  }
  if op.orientation != resOp.orientation {
    t.Errorf("orientations not equal: %v vs %v", op, resOp)
  }
  if !op.color.Equal(resOp.color) {
    t.Errorf("colors not equal: %v vs %v", op, resOp)
  }
}
