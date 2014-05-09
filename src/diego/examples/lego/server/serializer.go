package server

import "bytes"
import "diego/debug"

const legoDelim byte = ' '
const legoTerminator byte = '*'

type legoSerializable interface {
  serialize(*bytes.Buffer)
  deserialize(*bytes.Buffer)
}

func DeserializeTransactionsSince(buf *bytes.Buffer) (string, int64) {
  ns, err := buf.ReadString(legoDelim)
  ns = ns[:len(ns)-1]
  debug.EnsureNoError(err)

  id := deserializeInt64(buf)
  return ns, id
}
