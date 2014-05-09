package durable

import "os"
import "testing"
import "encoding/gob"

import "diego/debug"
import "diego/types"

type testTx struct {
  IdNum int64
}

func (tx *testTx) Id() int64 {
  return tx.IdNum
}

func (tx *testTx) SetId(id int64) {
  tx.IdNum = id
}

func expectNFilesAtPath(t *testing.T, n int, path string) {
  dir, err := os.Open(path)
  debug.EnsureNoError(err)

  names, err := dir.Readdirnames(0)
  debug.EnsureNoError(err)

  if len(names) != n {
    t.Errorf("Expected %d files; found %d: %v", n, len(names), names)
  }
}

func expectNFilePairs(t *testing.T, n int, tl *TransactionLogger) {
  if len(tl.dataFiles) != n {
    t.Errorf("Expected %d data files; found %d in map", n, len(tl.dataFiles))
  }
  if len(tl.indexFiles) != n {
    t.Errorf("Expected %d index files; found %d in map", n, len(tl.indexFiles))
  }
}

func expectNTransactions(t *testing.T, n int64, tl *TransactionLogger) {
  count := int64(0)
  callback := func(tr types.Transaction) {
    if tr.Id() != count {
      t.Errorf("Expected ID %d (in order) but got ID %d", count, tr.Id())
    }
    count++
  }

  tl.ReadAll(callback)
  if count != n {
    t.Errorf("Read %d transactions total, expected %d", count, n)
  }
}

var isSetUp bool
func setup() {
  if !isSetUp {
    gob.Register(&testTx{})
    isSetUp = true
  }
}

/*
All the tests below use the directory ./_test when starting from the root of the project.
*/

func TestAppendFew(t *testing.T) {
  const basePath = "../../../_test/durable/tx_few"
  os.RemoveAll(basePath)
  setup()

  tl := CreateTransactionLogger(basePath)
  numTestEntries := 198

  tl.assertValid()

  for i := 0; i < numTestEntries; i++ {
    tx := &testTx{int64(i)}
    tl.Append(tx)
    tl.assertValid()
  }

  expectNFilesAtPath(t, 2, basePath)
  expectNFilePairs(t, 1, tl)
  expectNTransactions(t, int64(numTestEntries), tl)
  tl.Close()
}

func TestAppendMany(t *testing.T) {
  const basePath = "../../../_test/durable/tx_many"
  os.RemoveAll(basePath)
  setup()

  tl := CreateTransactionLogger(basePath)
  numTestEntries := maxChunkLength + 2

  tl.assertValid()

  for i := 0; i < numTestEntries; i++ {
    tx := &testTx{int64(i)}
    tl.Append(tx)
    tl.assertValid()
  }

  expectNFilesAtPath(t, 4, basePath)
  expectNFilePairs(t, 2, tl)
  expectNTransactions(t, int64(numTestEntries), tl)
  tl.Close()
}

func TestCreateNextFiles(t *testing.T) {
  const basePath = "../../../_test/durable/create_next"
  os.RemoveAll(basePath)
  setup()

  tl := CreateTransactionLogger(basePath)
  tl.assertValid()

  // not part of API, should never happen in prod
  // testing internals
  tl.createNextFiles()
  tl.createNextFiles()

  tl.assertValid()

  expectNFilesAtPath(t, 6, basePath)
  expectNFilePairs(t, 3, tl)
  tl.Close()
}

func TestLoadExisting(t *testing.T) {
  const basePath = "../../../_test/durable/load_existing"
  os.RemoveAll(basePath)
  setup()

  tl := CreateTransactionLogger(basePath)
  tl.assertValid()

  firstLimit := 3
  for i := 0; i < firstLimit; i++ {
    tx := &testTx{int64(i)}
    tl.Append(tx)
    tl.assertValid()
  }

  expectNFilesAtPath(t, 2, basePath)
  expectNFilePairs(t, 1, tl)

  tl.Close()
  tl = CreateTransactionLogger(basePath)
  tl.assertValid()

  for i := firstLimit; i < maxChunkLength; i++ {
    tx := &testTx{int64(i)}
    tl.Append(tx)
    tl.assertValid()
  }

  // since we only added maxChunkLength transactions total
  // the new transaction logger should have still used the same files
  expectNFilesAtPath(t, 2, basePath)
  expectNFilePairs(t, 1, tl)
  expectNTransactions(t, maxChunkLength, tl)
  tl.Close()
}

func TestLoadNoneExisting(t *testing.T) {
  // ensure that when there are no transactions, ReadAll returns no transactions
  const basePath = "../../../_test/durable/load_none_existing"
  os.RemoveAll(basePath)
  setup()

  tl := CreateTransactionLogger(basePath)
  tl.assertValid()

  expectNFilesAtPath(t, 2, basePath)
  expectNFilePairs(t, 1, tl)

  tl.Close()
  tl = CreateTransactionLogger(basePath)
  tl.assertValid()

  expectNFilesAtPath(t, 2, basePath)
  expectNFilePairs(t, 1, tl)
  expectNTransactions(t, 0, tl)
  tl.Close()
}
