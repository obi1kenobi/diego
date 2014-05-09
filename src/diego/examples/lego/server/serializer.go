package server

import "bytes"
import "strconv"
import "diego/debug"

const legoDelim byte = ' '
const legoTerminator byte = '*'

type legoSerializable interface {
  serialize(*bytes.Buffer)
  deserialize(*bytes.Buffer)
}

func DeserializeTransactionsSince(buf *bytes.Buffer) (string, int64) {
  ns, err := buf.ReadString(legoDelim)
  debug.EnsureNoError(err)

  ns = ns[:len(ns)-1]
  val, err := buf.ReadString('\n')
  debug.EnsureNoError(err)

  id, err := strconv.ParseInt(val[:len(val)-1], 10, 64)
  debug.EnsureNoError(err)
  return ns, id
}
