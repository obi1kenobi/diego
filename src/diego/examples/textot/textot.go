package textot

import "strconv"
import "container/list"
import "bytes"
import "diego/debug"
import "diego/types"

type idObject struct {
  Tid int64
  Token types.RequestToken
}

/* Id - returns Id */
func (obj *idObject) Id() int64 {
  return obj.Tid
}

/* SetId - returns SetId */
func (obj *idObject) SetId(id int64) {
  obj.Tid = id
}

/* GetToken - returns RequestToken */
func (obj *idObject) GetToken() types.RequestToken {
  return obj.Token
}

type textState struct {
  idObject
  Str string
}

func makeState() types.State {
  result := new(textState)
  return result
}

func (ts textState) String() string {
  return "[" + strconv.Itoa(int(ts.Tid)) + "]" + ts.Str
}

/*
 Apply - apply transaction if it's recent enough, otherwise, cancel.
 */
func (ts *textState) Apply(action types.Transaction) (bool, types.Transaction) {
  debug.DPrintf(1, "Applying %v to %v.", action, ts)
  transform := action.(*textTransform)
  if ts.Tid != transform.Tid {
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
    if e.Value.(*textTransform).Tid < transform.Tid {
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
  OpType textOpType
  Str string
  N int
}

/*
 String - toString() for go
 */
func (op textOp) String() string {
  switch {
  case op.OpType == textSkipOp:
    return strconv.Itoa(op.N)
  case op.OpType == textInsertOp:
    return "'" + op.Str + "'"
  case op.OpType == textDeleteOp:
    return "{" + strconv.Itoa(op.N) + "}"
  default:
    return "INVALID"
  }
}

func (op *textOp) assertValid() {
  switch {
  case op.OpType == textSkipOp:
    debug.Assert(op.N > 0, "Skip op needs positive n: %d", op.N)
  case op.OpType == textInsertOp:
    debug.Assert(op.Str != "", "Insert op cannot have empty string")
  case op.OpType == textDeleteOp:
    debug.Assert(op.N > 0, "Delete op needs positive n: %d", op.N)
  default:
    debug.Assert(false, "Invalid op type %v", op.OpType)
  }
}


type textTransform struct {
  idObject
  Ops []textOp
}

func (tt textTransform) String() string {
  str := "{" + strconv.Itoa(int(tt.Tid)) + ": "
  for _, op := range tt.Ops {
    str += op.String() + ", "
  }
  return str + "}"
}

func (tt *textTransform) assertValid() {
  for _, op := range tt.Ops {
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
    if pos >= len(tt.Ops) {
      // At the end of the op sequence, all skips from here on out
      if n == -1 {
        return textOp{}, false
      }
      return textOp{OpType: textSkipOp, N: n}, false
    }
    op := tt.Ops[pos]
    var part textOp
    switch {
    case op.OpType == textSkipOp:
      if n == -1 || n >= op.N - offset {
        part = textOp{OpType: textSkipOp, N: op.N-offset}
        pos++
        offset = 0
        return part, true
      }
      offset += n
      return textOp{OpType: textSkipOp, N: n}, true
    case op.OpType == textInsertOp:
      part = textOp{OpType: textInsertOp, Str: op.Str[offset:]}
      pos++
      offset = 0
      return part, true
    case op.OpType == textDeleteOp:
      if n == -1 || n >= op.N - offset {
        part = textOp{OpType: textDeleteOp, N: op.N-offset}
        pos++
        offset = 0
        return part, true
      }
      offset += n
      return textOp{OpType: textDeleteOp, N: n}, true
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
  for _, op := range tt.Ops {
    switch {
    case op.OpType == textSkipOp:
      buf.WriteString(doc.Str[pos:pos+op.N])
      pos += op.N
    case op.OpType == textInsertOp:
      buf.WriteString(op.Str)
    case op.OpType == textDeleteOp:
      pos += op.N
    default:
      debug.Assert(false, "Found invalid op: %v", op)
    }
  }
  buf.WriteString(doc.Str[pos:])
  doc.Str = buf.String()
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
  for _, op := range ttBy.Ops {
    var length int
    var opSeg textOp
    switch {
    case op.OpType == textSkipOp:
      length = op.N
      for length > 0 {
        opSeg, _ = getNext(length)
        ttNew.Ops = append(ttNew.Ops, opSeg)
        if opSeg.OpType != textInsertOp {
          length -= op.N
        }
      }
    case op.OpType == textInsertOp:
      ttNew.Ops = append(ttNew.Ops, textOp{OpType: textSkipOp, N: len(op.Str)})
    case op.OpType == textDeleteOp:
      length = op.N
      for length > 0 {
        opSeg, _ = getNext(length)
        if opSeg.OpType == textInsertOp {
          ttNew.Ops = append(ttNew.Ops, opSeg)
        } else {
          length -= op.N
        }
      }
    }
  }
  for next, hasNext := getNext(-1); hasNext; next, hasNext = getNext(-1) {
    ttNew.Ops = append(ttNew.Ops, next)
  }
  ttNew.Tid = ttBy.Tid+1
  ttNew.Token = tt.Token
  ttNew.assertValid()
  return ttNew, nil
}
