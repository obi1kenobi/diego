package server

import "bytes"
import "strconv"
import "diego/debug"
import "diego/resolver"
import "fmt"

// Transactions
type LegoTransaction struct {
  id int64
  ops []interface{}
}

func (xa *LegoTransaction) SetId(id int64) {
  xa.id = id
}

func (xa *LegoTransaction) Id() int64 {
  return xa.id
}

func SerializeTransaction(ns string, xa *LegoTransaction, b *bytes.Buffer) {
  b.WriteString(ns)
  serializeInt64(xa.id, b)
  b.WriteByte('\n')
  for _, x := range xa.ops {
    s := x.(legoSerializable)
    s.serialize(b)
  }
  b.WriteByte(legoTerminator)
  b.WriteByte('\n')
}

func DeserializeTransaction(b *bytes.Buffer) (string, *LegoTransaction) {
  fmt.Printf("DeserializeTransaction: %v\n", b);

  xa := new(LegoTransaction)
  ns, err := b.ReadString(legoDelim)
  ns = ns[:len(ns)-1]
  debug.EnsureNoError(err)
  val, err := b.ReadString('\n')
  debug.EnsureNoError(err)
  xa.id, err = strconv.ParseInt(val[:len(val)-1], 10, 64)
  debug.EnsureNoError(err)

  for {
    val, err = b.ReadString('\n')
    debug.EnsureNoError(err)

    if val[0] == legoTerminator {
      return ns, xa
    }

    btemp := bytes.NewBufferString(val[:len(val)-1])
    btemp.WriteByte(legoDelim)
    val, err = btemp.ReadString(legoDelim)
    debug.EnsureNoError(err)
    val = val[:len(val)-1]
    switch val {
    case createBrick:
      op := new(LegoOpCreateBrick)
      op.deserialize(btemp)
      xa.ops = append(xa.ops, op)
    case deleteBrick:
      op := new(LegoOpDeleteBrick)
      // op.deserialize(btemp)
      xa.ops = append(xa.ops, op)
    case modifyBrickColor:
      op := new(LegoOpModifyBrickColor)
      // op.deserialize(btemp)
      xa.ops = append(xa.ops, op)
    case modifyBrickOrientation:
      op := new(LegoOpModifyBrickOrientation)
      // op.deserialize(btemp)
      xa.ops = append(xa.ops, op)
    case modifyBrickSize:
      op := new(LegoOpModifyBrickSize)
      // op.deserialize(btemp)
      xa.ops = append(xa.ops, op)
    default:
      debug.Assert(false, "Unknown op type: %v", val)
    }
  }
}

func SerializeServerResult(ns string, success bool, xas []resolver.Transaction,
                           b *bytes.Buffer) {
  if success {
    b.WriteByte('1')
  } else {
    b.WriteByte('0')
  }
  b.WriteByte('\n')

  SerializeTransactionSlice(ns, xas, b)
}

func SerializeTransactionSlice(ns string, xas []resolver.Transaction, b *bytes.Buffer) {
  for _, x := range xas {
    SerializeTransaction(ns, x.(*LegoTransaction), b)
  }
}
