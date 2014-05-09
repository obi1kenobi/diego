package textot

import "strconv"
import "container/list"
import "bytes"
import "diego/debug"
import "diego/types"

type idObject struct {
  id int64
  token types.RequestToken
}

/* Id - returns Id */
func (obj *idObject) Id() int64 {
  return obj.id
}

/* SetId - returns SetId */
func (obj *idObject) SetId(id int64) {
  obj.id = id
}

/* GetToken - returns RequestToken */
func (obj *idObject) GetToken() types.RequestToken {
  return obj.token
}

type textState struct {
  idObject
  str string
}

func makeState() types.State {
  result := new(textState)
  return result
}

func (ts textState) String() string {
  return "[" + strconv.Itoa(int(ts.id)) + "]" + ts.str
}

/*
 Apply - apply transaction if it's recent enough, otherwise, cancel.
 */
func (ts *textState) Apply(action types.Transaction) (bool, types.Transaction) {
  debug.DPrintf(1, "Applying %v to %v.", action, ts)
  transform := action.(*textTransform)
  if ts.id != transform.id {
    return false, action
  }
  transform.assertValid()
  err := transform.applyOn(ts)
  if err != nil {
    debug.Assert(false, "Could not apply transform %v on state %v: %v", action, ts, err)
  }
  return true, action
}

/*
 Resolve - Update transform through the log, then return final transform
 */
func (ts *textState) Resolve(ancestorState *types.State, log *list.List,
                             current types.Transaction) (bool, types.Transaction) {
  debug.DPrintf(1, "Resolving %v from %v to %v.", current, log.Front().Value, log.Back().Value)
  var err error
  transform := current.(*textTransform)
  for e := log.Front(); e != nil; e = e.Next() {
    if e.Value.(*textTransform).id < transform.id {
      continue
    }
    transform, err = transform.transformWith(e.Value.(*textTransform))
    if err != nil {
      debug.Assert(false, "Resolve failed when transforming %v with %v: %v.", transform, e.Value, err)
    }
  }
  return true, transform
}


type textOpType int
const (
  textSkipOp   textOpType = 0
  textInsertOp textOpType = 1
  textDeleteOp textOpType = 2
)

type textOp struct {
  opType textOpType
  str string
  n int
}

/*
 String - toString() for go
 */
func (op textOp) String() string {
  switch {
  case op.opType == textSkipOp:
    return strconv.Itoa(op.n)
  case op.opType == textInsertOp:
    return "'" + op.str + "'"
  case op.opType == textDeleteOp:
    return "{" + strconv.Itoa(op.n) + "}"
  default:
    return "INVALID"
  }
}

func (op *textOp) assertValid() {
  switch {
  case op.opType == textSkipOp:
    debug.Assert(op.n > 0, "Skip op needs positive n: %d", op.n)
  case op.opType == textInsertOp:
    debug.Assert(op.str != "", "Insert op cannot have empty string")
  case op.opType == textDeleteOp:
    debug.Assert(op.n > 0, "Delete op needs positive n: %d", op.n)
  default:
    debug.Assert(false, "Invalid op type %v", op.opType)
  }
}


type textTransform struct {
  idObject
  ops []textOp
}

func (tt textTransform) String() string {
  str := "{" + strconv.Itoa(int(tt.id)) + ": "
  for _, op := range tt.ops {
    str += op.String() + ", "
  }
  return str + "}"
}

func (tt *textTransform) assertValid() {
  for _, op := range tt.ops {
    op.assertValid()
  }
}

/*
 makeIterators - produce iterator function getNext(length)
 */
func (tt *textTransform) makeIterator () (getNext func(int) (textOp, bool)) {
  var pos, offset int

  // Take up to length n from front of op. If n is -1, then return entire op.
  // insert components are not separated.
  getNext = func(n int) (textOp, bool) {
    if pos >= len(tt.ops) {
      // At the end of the op sequence, all skips from here on out
      if n == -1 {
        return textOp{}, false
      }
      return textOp{opType: textSkipOp, n: n}, false
    }
    op := tt.ops[pos]
    var part textOp
    switch {
    case op.opType == textSkipOp:
      if n == -1 || n >= op.n - offset {
        part = textOp{opType: textSkipOp, n: op.n-offset}
        pos++
        offset = 0
        return part, true
      }
      offset += n
      return textOp{opType: textSkipOp, n: n}, true
    case op.opType == textInsertOp:
      part = textOp{opType: textInsertOp, str: op.str[offset:]}
      pos++
      offset = 0
      return part, true
    case op.opType == textDeleteOp:
      if n == -1 || n >= op.n - offset {
        part = textOp{opType: textDeleteOp, n: op.n-offset}
        pos++
        offset = 0
        return part, true
      }
      offset += n
      return textOp{opType: textDeleteOp, n: n}, true
    default:
      debug.Assert(false, "got invalid op during getNext: %v", op)
    }
    return textOp{}, false
  }
  return
}

/*
 applyOn - apply the transform to the given document in place. Should always succeed.
 */
func (tt *textTransform) applyOn (doc *textState) error {
  var pos int
  var buf bytes.Buffer
  for _, op := range tt.ops {
    switch {
    case op.opType == textSkipOp:
      buf.WriteString(doc.str[pos:pos+op.n])
      pos += op.n
    case op.opType == textInsertOp:
      buf.WriteString(op.str)
    case op.opType == textDeleteOp:
      pos += op.n
    default:
      debug.Assert(false, "Found invalid op: %v", op)
    }
  }
  buf.WriteString(doc.str[pos:])
  doc.str = buf.String()
  return nil
}

/*
 transformWith - apply the given transform onto this transform such that the new
 transform can be applied after the given transform
 */
func (tt *textTransform) transformWith (ttBy *textTransform) (*textTransform, error) {
  debug.DPrintf(1, "transforming %v by %v", tt, ttBy)
  ttNew := &textTransform{}
  getNext := tt.makeIterator()
  for _, op := range ttBy.ops {
    var length int
    var opSeg textOp
    switch {
    case op.opType == textSkipOp:
      length = op.n
      for length > 0 {
        opSeg, _ = getNext(length)
        ttNew.ops = append(ttNew.ops, opSeg)
        if opSeg.opType != textInsertOp {
          length -= op.n
        }
      }
    case op.opType == textInsertOp:
      ttNew.ops = append(ttNew.ops, textOp{opType: textSkipOp, n: len(op.str)})
    case op.opType == textDeleteOp:
      length = op.n
      for length > 0 {
        opSeg, _ = getNext(length)
        if opSeg.opType == textInsertOp {
          ttNew.ops = append(ttNew.ops, opSeg)
        } else {
          length -= op.n
        }
      }
    }
  }
  for next, hasNext := getNext(-1); hasNext; next, hasNext = getNext(-1) {
    ttNew.ops = append(ttNew.ops, next)
  }
  ttNew.id = ttBy.id+1
  ttNew.token = tt.token
  ttNew.assertValid()
  return ttNew, nil
}
