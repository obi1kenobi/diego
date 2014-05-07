package server

import "bytes"

const legoDelim byte = ' '
const legoTerminator byte = '*'

type legoSerializable interface {
  serialize(*bytes.Buffer)
  deserialize(*bytes.Buffer)
}
