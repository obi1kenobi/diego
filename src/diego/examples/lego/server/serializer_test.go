package server

import "bytes"
import "testing"

func TestSerializeCreateOp(t *testing.T) {
  op := &LegoOp { LegoOpCreateBrick, 0, MakeVec3i(0, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) }
  ops := []*LegoOp { op }
  xa := &LegoTransaction{Ops: ops}
  ns := "testNs"

  b := new(bytes.Buffer)
  SerializeTransaction(ns, xa, b)

  resNs, resXa := DeserializeTransaction(b)

  if ns != resNs {
    t.Errorf("expected ns=%s got resNs=%s", ns, resNs)
  }

  if len(xa.Ops) != len(resXa.Ops) {
    t.Errorf("expected len(xa.ops)=%d got len(resXa.ops)=%d", len(xa.Ops), len(resXa.Ops))
  }

  resOp := resXa.Ops[0]
  if !op.Position.Equal(resOp.Position) {
    t.Errorf("positions not equal: %v vs %v", op, resOp)
  }
  if !op.Size.Equal(resOp.Size) {
    t.Errorf("sizes not equal: %v vs %v", op, resOp)
  }
  if op.Orientation != resOp.Orientation {
    t.Errorf("orientations not equal: %v vs %v", op, resOp)
  }
  if !op.Color.Equal(resOp.Color) {
    t.Errorf("colors not equal: %v vs %v", op, resOp)
  }
}
