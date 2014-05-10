package kvstore

import "os"
import "time"
import "strconv"
import "testing"

import "diego/resolver"
import "diego/types"

func recentTransactionBench(trailingDist int, b *testing.B) {
  rs := resolver.CreateResolver(makeState, trailingDist, "")
  recentTransactionHarness(rs, b)
  rs.Close()
}

func durableRecentTransactionBench(trailingDist int, b *testing.B) {
  const basePath = "../../../../_test/bench_durable/"
  path := basePath + "recent_" + strconv.Itoa(trailingDist)

  err := os.RemoveAll(basePath)
  for err != nil {
    time.Sleep(50 * time.Millisecond)
    err = os.RemoveAll(basePath)
  }

  rs := resolver.CreateResolver(makeState, trailingDist, path)
  recentTransactionHarness(rs, b)
  rs.Close()
}

func recentTransactionHarness(rs *resolver.Resolver, b *testing.B) {
  xas := make([]types.Transaction, b.N)
  accept := make([]bool, b.N)
  nt := types.MakeRequestTokenGenerator(0)
  for i := 0; i < b.N; i++ {
    xas[i] = &lwwSetOp{int64(i), nt(), "a", "b"}
  }
  b.ResetTimer()

  for i := 0; i < b.N; i++ {
    accept[i], _ = rs.SubmitTransaction(xas[i])
  }

  b.StopTimer()
  for i := 0; i < b.N; i++ {
    if !accept[i] {
      b.Fatalf("Transaction %+v failed to apply.", xas[i])
    }
  }
}

func fullLogReadBench(trailingDist int, b *testing.B) {
  rs := resolver.CreateResolver(makeState, trailingDist, "")
  fullLogReadHarness(rs, trailingDist, b)
  rs.Close()
}

func durableFullLogReadBench(trailingDist int, b *testing.B) {
  const basePath = "../../../../_test/bench_durable/"
  path := basePath + "full_" + strconv.Itoa(trailingDist)

  err := os.RemoveAll(basePath)
  for err != nil {
    time.Sleep(50 * time.Millisecond)
    err = os.RemoveAll(basePath)
  }

  rs := resolver.CreateResolver(makeState, trailingDist, path)
  fullLogReadHarness(rs, trailingDist, b)
  rs.Close()
}

func fullLogReadHarness(rs *resolver.Resolver, trailingDist int, b *testing.B) {
  xas := make([]types.Transaction, b.N)
  accept := make([]bool, b.N)
  nt := types.MakeRequestTokenGenerator(0)
  max := func(a, b int64) int64 {
    if a < b {
      return b
    }
    return a
  }

  for i := 0; i < b.N; i++ {
    xas[i] = &testAndSetOp{max(0, int64(i-trailingDist)), nt(), strconv.Itoa(i), "b"}
  }
  b.ResetTimer()

  for i := 0; i < b.N; i++ {
    accept[i], _ = rs.SubmitTransaction(xas[i])
  }

  b.StopTimer()
  for i := 0; i < b.N; i++ {
    if !accept[i] {
      b.Fatalf("Transaction %+v failed to apply.", xas[i])
    }
  }
}

func BenchmarkRecentTransactionTrailing50(b *testing.B) {
  const trailing = 50
  recentTransactionBench(trailing, b)
}

func BenchmarkRecentTransactionTrailing250(b *testing.B) {
  const trailing = 250
  recentTransactionBench(trailing, b)
}

func BenchmarkRecentTransactionTrailing1000(b *testing.B) {
  const trailing = 1000
  recentTransactionBench(trailing, b)
}

func BenchmarkDurableRecentTransactionTrailing50(b *testing.B) {
  const trailing = 50
  durableRecentTransactionBench(trailing, b)
}

func BenchmarkDurableRecentTransactionTrailing250(b *testing.B) {
  const trailing = 250
  durableRecentTransactionBench(trailing, b)
}

func BenchmarkDurableRecentTransactionTrailing1000(b *testing.B) {
  const trailing = 1000
  durableRecentTransactionBench(trailing, b)
}

func BenchmarkFullLogReadTrailing50(b *testing.B) {
  const trailing = 50
  fullLogReadBench(trailing, b)
}

func BenchmarkFullLogReadTrailing250(b *testing.B) {
  const trailing = 250
  fullLogReadBench(trailing, b)
}

func BenchmarkFullLogReadTrailing1000(b *testing.B) {
  const trailing = 1000
  fullLogReadBench(trailing, b)
}

func BenchmarkDurableFullLogReadTrailing50(b *testing.B) {
  const trailing = 50
  durableFullLogReadBench(trailing, b)
}

func BenchmarkDurableFullLogReadTrailing250(b *testing.B) {
  const trailing = 250
  durableFullLogReadBench(trailing, b)
}

func BenchmarkDurableFullLogReadTrailing1000(b *testing.B) {
  const trailing = 1000
  durableFullLogReadBench(trailing, b)
}
