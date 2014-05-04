package durable

import "os"
import "testing"
import "encoding/gob"

type testTx struct {
  IdNum int64
}

func (tx *testTx) Id() int64 {
  return tx.IdNum
}

func (tx *testTx) SetId(id int64) {
  tx.IdNum = id
}

func TestAppendFew(t *testing.T) {
  const basePath = "../../../_test/durable/tx_few"
  os.RemoveAll(basePath)
  gob.Register(testTx{})

  tl := CreateTransactionLogger(basePath)

  tl.assertValid()

  for i := 0; i < 5; i++ {
    tx := &testTx{int64(i)}
    tl.Append(tx)
    tl.assertValid()
  }

  dir, err := os.Open(basePath)
  ensureNoError(err)

  names, err := dir.Readdirnames(0)
  ensureNoError(err)

  if len(names) != 2 {
    t.Errorf("Expected 2 files, found %v", names)
  }
}

func TestAppendMany(t *testing.T) {
  const basePath = "../../../_test/durable/tx_many"
  os.RemoveAll(basePath)
  gob.Register(testTx{})

  tl := CreateTransactionLogger(basePath)

  tl.assertValid()

  for i := 0; i < maxChunkLength + 2; i++ {
    tx := &testTx{int64(i)}
    tl.Append(tx)
    tl.assertValid()
  }

  dir, err := os.Open(basePath)
  ensureNoError(err)

  names, err := dir.Readdirnames(0)
  ensureNoError(err)

  if len(names) != 4 {
    t.Errorf("Expected 4 files, found %v", names)
  }
}

func TestCreateNextFiles(t *testing.T) {
  const basePath = "../../../_test/durable/createnext"
  os.RemoveAll(basePath)
  tl := CreateTransactionLogger(basePath)
  tl.assertValid()

  // not part of API
  // testing internals
  tl.createNextFiles()
  tl.createNextFiles()

  tl.assertValid()

  dir, err := os.Open(basePath)
  ensureNoError(err)

  names, err := dir.Readdirnames(0)
  ensureNoError(err)

  if len(names) != 6 {
    t.Errorf("Expected 6 files, found %v", names)
  }
}
