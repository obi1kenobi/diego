package kvstore

import "strconv"
import "testing"

import "diego/resolver"
import "diego/types"

func recentTransactionHarness(trailingDist int, b *testing.B) {
  rs := resolver.CreateResolver(makeState, trailingDist, "")
  xas := make([]types.Transaction, b.N)
  accept := make([]bool, b.N)
  for i := 0; i < b.N; i++ {
    xas[i] = &lwwSetOp{int64(i), "a", "b"}
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

func fullLogReadHarness(trailingDist int, b *testing.B) {
  rs := resolver.CreateResolver(makeState, trailingDist, "")
  xas := make([]types.Transaction, b.N)
  accept := make([]bool, b.N)
  max := func(a, b int64) int64 {
    if a < b {
      return b
    }
    return a
  }

  for i := 0; i < b.N; i++ {
    xas[i] = &testAndSetOp{max(0, int64(i-trailingDist)), strconv.Itoa(i), "b"}
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
  recentTransactionHarness(trailing, b)
}

func BenchmarkRecentTransactionTrailing250(b *testing.B) {
  const trailing = 250
  recentTransactionHarness(trailing, b)
}

func BenchmarkRecentTransactionTrailing1000(b *testing.B) {
  const trailing = 1000
  recentTransactionHarness(trailing, b)
}

func BenchmarkFullLogReadTrailing50(b *testing.B) {
  const trailing = 50
  fullLogReadHarness(trailing, b)
}

func BenchmarkFullLogReadTrailing250(b *testing.B) {
  const trailing = 250
  fullLogReadHarness(trailing, b)
}

func BenchmarkFullLogReadTrailing1000(b *testing.B) {
  const trailing = 1000
  fullLogReadHarness(trailing, b)
}
