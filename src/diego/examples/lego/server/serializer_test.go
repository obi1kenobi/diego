package server

import "bytes"
import "testing"
import "diego/types"

func TestSerializeCreateOp(t *testing.T) {
  op := &LegoOp { LegoOpCreateBrick, 0, MakeVec3i(0, 0, 0), MakeVec3i(2, 2, 1), BrickOrientationNorth, MakeVec3f(1, 0, 0) }
  ops := []*LegoOp { op }
  xa := &LegoTransaction{Tid: int64(109417393), Token: types.RequestToken{int64(834273938), int64(193743)}, Ops: ops}
  ns := "testNs"

  b := new(bytes.Buffer)
  SerializeTransaction(ns, xa, b)

  resNs, resXa := DeserializeTransaction(b)

  if ns != resNs {
    t.Errorf("expected ns=%s got resNs=%s", ns, resNs)
  }
  if resXa.Id() != xa.Id() {
    t.Errorf("id's not equal: %d %d", resXa.Id(), xa.Id())
  }
  if resXa.GetToken() != xa.GetToken() {
    t.Errorf("tokens not equal: %v %v", resXa.GetToken(), xa.GetToken())
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
