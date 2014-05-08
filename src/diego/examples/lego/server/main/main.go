package main

import "bytes"
import "io/ioutil"
import "net/http"
import "diego/debug"
import "diego/core"
import "diego/examples/lego/server"
import "fmt"

var dc *core.DiegoCore

const address = ":8080"

func main() {
  fmt.Printf("Creating diego...\n")
  dc = core.CreateDiegoCore(500, server.MakeState)
  fmt.Printf("Done.\n")

  fmt.Printf("Launching httpd...\n")
  http.HandleFunc("/lego", legoConnectionHandler)
  http.ListenAndServe(address, nil)
  fmt.Printf("Done.\n")
}

func legoConnectionHandler(w http.ResponseWriter, r *http.Request) {
  fmt.Printf("Begin callback\n")

  b, err := ioutil.ReadAll(r.Body)
  debug.EnsureNoError(err)

  if len(b) == 0 {
    return
  }

  buf := bytes.NewBuffer(b)

  fmt.Printf("Got message: %v\n", buf)
  ns, xa := server.DeserializeTransaction(buf)
  success, xas := dc.SubmitTransaction(ns, xa)

  resultBuf := &bytes.Buffer{}
  server.SerializeServerResult(ns, success, xas, resultBuf)
  _, err = resultBuf.WriteTo(w)
  debug.EnsureNoError(err)

  fmt.Printf("End callback\n")
}
