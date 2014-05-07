package main

import "bytes"
import "io/ioutil"
import "net/http"
import "diego/debug"
import "diego/core"
import "diego/examples/lego/server"

var dc *core.DiegoCore

const address = ":8080"

func main() {
  dc = core.CreateDiegoCore(500, server.MakeState)
  http.HandleFunc("/lego", legoConnectionHandler)
  http.ListenAndServe(address, nil)
}

func legoConnectionHandler(w http.ResponseWriter, r *http.Request) {
  b, err := ioutil.ReadAll(r.Body)
  debug.EnsureNoError(err)

  if len(b) == 0 {
    return
  }

  buf := bytes.NewBuffer(b)
  ns, xa := server.DeserializeTransaction(buf)
  success, xas := dc.SubmitTransaction(ns, xa)

  resultBuf := &bytes.Buffer{}
  server.SerializeServerResult(ns, success, xas, resultBuf)
  _, err = resultBuf.WriteTo(w)
  debug.EnsureNoError(err)
}
