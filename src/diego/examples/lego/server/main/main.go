package main

import "bytes"
import "io/ioutil"
import "net/http"
import "diego/debug"
import "diego/core"
import "diego/examples/lego/server"
import "encoding/gob"
import "fmt"

var dc *core.DiegoCore

const address = ":8080"

func main() {
  gob.Register(&server.LegoTransaction{})

  debug.DPrintf(0, "Creating diego...")
  dc = core.CreateDiegoCore(10000, server.MakeState, "LegoStorage")
  debug.DPrintf(0, "Done.")

  debug.DPrintf(0, "Launching httpd...")
  http.HandleFunc("/lego", legoConnectionHandler)
  http.ListenAndServe(address, nil)
  debug.DPrintf(0, "Done.")
}

func legoConnectionHandler(w http.ResponseWriter, r *http.Request) {
  debug.DPrintf(3, "Begin callback")

  b, err := ioutil.ReadAll(r.Body)
  debug.EnsureNoError(err)

  if len(b) == 0 {
    return
  }

  buf := bytes.NewBuffer(b)

  debug.DPrintf(0, "Got message: %v\n", buf)

  command, err := buf.ReadString('\n')
  debug.EnsureNoError(err)

  command = command[:len(command)-1]
  resultBuf := &bytes.Buffer{}

  switch command {
  case "Submit":
    ns, xa := server.DeserializeTransaction(buf)
    success, xas := dc.SubmitTransaction(ns, xa)
    server.SerializeServerResult(ns, success, xas, resultBuf)
  case "TransactionsSince":
    ns, id := server.DeserializeTransactionsSince(buf)
    xas, _ := dc.TransactionsSinceId(ns, id)
    server.SerializeTransactionSlice(ns, xas, resultBuf)
    _, err = resultBuf.WriteTo(w)
    debug.EnsureNoError(err)
  default:
    fmt.Printf("ERROR: Unrecognized command: '%s'", command)
  }

  _, err = resultBuf.WriteTo(w)
  debug.EnsureNoError(err)

  debug.DPrintf(3, "End callback\n")
}
