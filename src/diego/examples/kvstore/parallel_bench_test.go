package kvstore

import "strconv"
import "testing"
import "math/rand"

import "diego/types"
import "diego/tests"
import "diego/core"

const noLogging = ""
const logging = "../../../../_test/bench_kvstore"

func makeKVCoreFactory(trailingDistance int, durablePath string) func()*core.DiegoCore{
  return func()*core.DiegoCore {
    return core.CreateDiegoCore(trailingDistance, makeState, durablePath)
  }
}

func makeKVTestTransaction (token types.RequestToken, rand *rand.Rand) types.Transaction {
  return &testAndSetOp{OpCore{0, token, strconv.Itoa(rand.Intn(10)), "b"}}
}

func BenchmarkParallelBasic(b *testing.B) {
  config := tests.BenchmarkConfig{}
  config.DurablePath          = noLogging
  config.MakeCore             = makeKVCoreFactory(250, config.DurablePath)
  config.MakeTransaction      = makeKVTestTransaction
  config.NumNamespaces        = 2
  config.RandomNamespacing    = true
  config.NumClients           = 1
  config.NumRequestsPerClient = b.N
  tests.ParallelClientBenchmarks(config, b)
}
