package tests

import "math/rand"
import "os"
import "strconv"
import "testing"
import "time"
import "diego/types"
import "diego/debug"
import "diego/core"

type ClientRequest struct {
  Ns string
  Xa types.Transaction
}

type ClientFunc func(*core.DiegoCore, chan<- int, <-chan int, chan<- int, *testing.B)

type BenchmarkConfig struct {
  DurablePath string
  MakeCore func() *core.DiegoCore
  MakeTransaction func(types.RequestToken, *rand.Rand) types.Transaction
  NumNamespaces int
  RandomNamespacing bool
  NumClients int
  NumRequestsPerClient int
}

func ParallelClientBenchmarks(config BenchmarkConfig, b *testing.B) {
  err := os.RemoveAll(config.DurablePath)
  for err != nil {
    time.Sleep(50 * time.Millisecond)
    err = os.RemoveAll(config.DurablePath)
  }
  os.MkdirAll(config.DurablePath, 0777)

  core := config.MakeCore()
  clients := MakeParallelBenchmarkClients (core, config, b)
  RunParallelClientBenchmarks(core, clients, b)
}

func RunParallelClientBenchmarks(core *core.DiegoCore,
                                 clients []ClientFunc,
                                 b *testing.B) {
  readyChan := make(chan int)
  startChan := make(chan int)
  endChan := make(chan int)

  for _, client := range clients {
    go client(core, readyChan, startChan, endChan, b)
    <-readyChan
  }

  close(startChan)
  b.ResetTimer()

  for clientsLeft := len(clients); clientsLeft > 0; clientsLeft-- {
    <-endChan
  }

  b.StopTimer()
}

func MakeParallelClient(clientId int, requests []ClientRequest) ClientFunc {
  return func(diego *core.DiegoCore, ready chan<- int, start <-chan int, stop chan<- int, b *testing.B) {
    stateIds := make(map[string]int64)

    ready <- clientId
    <-start

    for _, request := range requests {
      request.Xa.SetId(stateIds[request.Ns])
      success, newXas := diego.SubmitTransaction(request.Ns, request.Xa)
      for !success {
        stateIds[request.Ns], _ = diego.CurrentStateId(request.Ns)
        request.Xa.SetId(stateIds[request.Ns])
        success, newXas = diego.SubmitTransaction(request.Ns, request.Xa)
      }
      debug.Assert(len(newXas) > 0, "%v, %v", newXas,  success)
      stateIds[request.Ns] = newXas[len(newXas)-1].Id()
    }

    stop <- clientId
  }
}

func MakeParallelBenchmarkClients(core *core.DiegoCore, cf BenchmarkConfig,
                               b *testing.B) []ClientFunc {
  rand := rand.New(rand.NewSource(int64(1)))
  nsMap := make(map[int]string)
  for i := 0; i < cf.NumNamespaces; i++ {
    nsMap[i] = "ns-" + strconv.Itoa(i)
  }
  clients := make([]ClientFunc, cf.NumClients)

  for i := 0; i < cf.NumClients; i++ {
    nt := types.MakeRequestTokenGenerator(int64(i))
    ops := make([]ClientRequest, cf.NumRequestsPerClient)
    for j := 0; j < cf.NumRequestsPerClient; j++ {
      request := ClientRequest{}
      request.Xa = cf.MakeTransaction(nt(), rand)
      if cf.RandomNamespacing {
        request.Ns = nsMap[rand.Intn(cf.NumNamespaces)]
      } else {
        request.Ns = nsMap[i]
      }
      ops[j] = request
    }
    clients[i] = MakeParallelClient(i, ops)
  }
  return clients
}
