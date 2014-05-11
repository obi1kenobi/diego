package kvstore

import "os"
import "testing"
import "encoding/gob"

import "diego/debug"
import "diego/core"
import "diego/types"

var isRegistered bool
func registerOps() {
  if !isRegistered {
    gob.Register(&lwwSetOp{})
    gob.Register(&appendOp{})
    gob.Register(&pessimisticSetOp{})
    gob.Register(&testAndSetOp{})
    gob.Register(&flipflopAddOp{})
    isRegistered = true
  }
}

func TestBasicCrashRestore (t *testing.T) {
  const basePath = "../../../../_test/core_durable/basic"
  os.RemoveAll(basePath)
  os.MkdirAll(basePath, 0777)
  registerOps()
  nt := types.MakeRequestTokenGenerator(0)

  c1 := core.CreateDiegoCore(20, makeState, basePath)
  for i := int64(0); i < 10; i++ {
    success, _ := c1.SubmitTransaction("foo", &appendOp{i, nt(), "a", "b"})
    debug.Assert(success, "transaction set 1-%d did not succeed!", i)
  }
  for i := int64(0); i < 30; i++ {
    success, _ := c1.SubmitTransaction("bar", &appendOp{i, nt(), "c", "d"})
    debug.Assert(success, "transaction set 2-%d did not succeed!", i)
  }
  core.KillCore(c1)
  c2 := core.CreateDiegoCore(20, makeState, basePath)

  core.AssertCoresEqual(c1, c2, stateEquals, appendOpsEqual)
}

func TestNewNamespaceCrashRestore (t *testing.T) {
  const basePath = "../../../../_test/core_durable/new"
  os.RemoveAll(basePath)
  os.MkdirAll(basePath, 0777)
  registerOps()
  nt := types.MakeRequestTokenGenerator(0)

  c1 := core.CreateDiegoCore(20, makeState, basePath)
  for i := int64(0); i < 10; i++ {
    success, _ := c1.SubmitTransaction("foo", &appendOp{i, nt(), "a", "b"})
    debug.Assert(success, "transaction set 1-%d did not succeed!", i)
  }
  for i := int64(0); i < 30; i++ {
    success, _ := c1.SubmitTransaction("bar", &appendOp{i, nt(), "c", "d"})
    debug.Assert(success, "transaction set 2-%d did not succeed!", i)
  }
  success, _ := c1.SubmitTransaction("eggs", &appendOp{0, nt(), "c", "d"})
  debug.Assert(success, "transaction set 3-%d did not succeed!", 0)
  core.KillCore(c1)
  c2 := core.CreateDiegoCore(20, makeState, basePath)

  core.AssertCoresEqual(c1, c2, stateEquals, appendOpsEqual)
}

func TestDeleteNamespaceCrashRestore (t *testing.T) {
  const basePath = "../../../../_test/core_durable/remove"
  os.RemoveAll(basePath)
  os.MkdirAll(basePath, 0777)
  registerOps()
  nt := types.MakeRequestTokenGenerator(0)

  c1 := core.CreateDiegoCore(20, makeState, basePath)
  for i := int64(0); i < 10; i++ {
    success, _ := c1.SubmitTransaction("foo", &appendOp{i, nt(), "a", "b"})
    debug.Assert(success, "transaction set 1-%d did not succeed!", i)
  }
  for i := int64(0); i < 30; i++ {
    success, _ := c1.SubmitTransaction("bar", &appendOp{i, nt(), "c", "d"})
    debug.Assert(success, "transaction set 2-%d did not succeed!", i)
  }
  c1.RemoveNamespace("bar")
  core.KillCore(c1)
  c2 := core.CreateDiegoCore(20, makeState, basePath)

  core.AssertCoresEqual(c1, c2, stateEquals, appendOpsEqual)
}
