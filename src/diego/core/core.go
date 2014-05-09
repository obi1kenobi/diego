package core

import "os"
import "path"
import "bytes"
import "io/ioutil"
import "encoding/base64"

import "diego/debug"
import "diego/types"
import "diego/resolver"
import "diego/namespace"

/*
AssertCoresEqual - assert that two diego cores have the same information.
Useful for durability tests
*/
func AssertCoresEqual (dc1, dc2 *DiegoCore) {
  namespace.AssertNamespacesEqual(dc1.nsManager, dc2.nsManager)
}

/*
KillCore - lock down a core - should be equivalent to crasing
*/
func KillCore (dc *DiegoCore) {
  namespace.CloseAllNamespaces(dc.nsManager)
}

/*
DiegoCore - the main object of the Diego conflict resolution framework.
*/
type DiegoCore struct {
  nsManager *namespace.NamespaceManager
  trailingDistance int
  makeState func()types.State
  durablePath string
}

/*
CreateDiegoCore - factory method to make a diego core object. If durablePath is "", no durable copy of transactions will be recorded. Otherwise, the path must
  point to a directory which is either empty or the durable path of a previous diego core.
*/
func CreateDiegoCore(trailingDistance int, makeState func()types.State, durablePath string) *DiegoCore {
  dc := new(DiegoCore)
  dc.trailingDistance = trailingDistance

  if dc.trailingDistance <= 0 {
    dc.trailingDistance = resolver.DefaultTrailingDistance
  }

  dc.makeState = makeState
  dc.nsManager = namespace.CreateNamespaceManager()

  dc.durablePath = durablePath
  if dc.durablePath != "" {
    dc.loadDurableNamespaces()
  }

  return dc
}

func unbase64(s string) (string, error) {
  b, err := base64.URLEncoding.DecodeString(s)
  if err != nil {
    return "", err
  }
  buf := bytes.NewBuffer(b)
  return buf.String(), nil
}

func (dc *DiegoCore) loadDurableNamespaces() {
  files, err := ioutil.ReadDir(dc.durablePath)
  debug.EnsureNoError(err)
  for _, f := range files {
    if f.IsDir() {
      ns, err := unbase64(f.Name())
      if err != nil {
        debug.DPrintf(0, "Couldn't load namespace with non-base64 name %s", f.Name())
      } else {
        debug.DPrintf(1, "Reading in namespace %s.", ns)
        dc.robustGetNamespace(ns)
      }
    }
  }
}

func (dc *DiegoCore) makeResolverDurablePath(ns string) string {
  if dc.durablePath == "" {
    return ""
  }

  buf := bytes.NewBufferString(ns)
  dirname := base64.URLEncoding.EncodeToString(buf.Bytes())
  return path.Join(dc.durablePath, dirname)
}

func (dc *DiegoCore) robustGetNamespace(ns string) *resolver.Resolver {
  rs, ok := dc.nsManager.GetNamespace(ns)
  if !ok {
    rs = resolver.CreateResolver(dc.makeState, dc.trailingDistance, dc.makeResolverDurablePath(ns))
    for !ok {
      ok = dc.nsManager.CreateNamespace(ns, rs)
      if !ok {
        // the create failed -- we lost the race to create a new namespace
        // try getting the namespace again
        // must try in a for-loop, because someone may have deleted it in the meantime
        rs, ok = dc.nsManager.GetNamespace(ns)
      }
    }
  }
  return rs
}

/*
RemoveNamespace - deletes the specified namespace and its durable log.
*/
func (dc *DiegoCore) RemoveNamespace(ns string) {
  if dc.nsManager.RemoveNamespace(ns) && dc.durablePath != "" {
    dir := dc.makeResolverDurablePath(ns)
    err := os.RemoveAll(dir)
    debug.EnsureNoError(err)
  }
}

/*
SubmitTransaction - submits the specified transaction to the specified namespace.
  The namespace is created if it doesn't exist.
  Return value: transaction success + all transactions that the client hasn't seen yet

  Safe for concurrent use.
*/
func (dc *DiegoCore) SubmitTransaction(ns string, t types.Transaction) (bool, []types.Transaction) {
  rs := dc.robustGetNamespace(ns)
  tid := t.Id()
  ok, _ := rs.SubmitTransaction(t)
  _, transactions := rs.TransactionsSinceId(tid)
  return ok, transactions
}

/*
TransactionsSinceId - gets all transactions in the given namespace that happened at or
  after the specified state id. If the namespace doesn't exist, the second return argument
  will be false. See types.TransactionsSinceId for details.

  Safe for concurrent use.
*/
func (dc *DiegoCore) TransactionsSinceId(ns string, id int64) ([]types.Transaction, bool) {
  rs, ok := dc.nsManager.GetNamespace(ns)
  if !ok {
    return nil, false
  }

  _, transactions := rs.TransactionsSinceId(id)
  return transactions, true
}

/*
CurrentStateId - gets the current state id in the given namespace. If the namespace doesn't exist,
  the second return argument will be false. See resolver.CurrentStateId for details.

  Safe for concurrent use.
*/
func (dc *DiegoCore) CurrentStateId(ns string) (int64, bool) {
  rs, ok := dc.nsManager.GetNamespace(ns)
  if !ok {
    return 0, false
  }

  sid := rs.CurrentStateId()
  return sid, true
}

/*
CurrentState - calls the callback with the current state for the given namespace. If the namespace
  doesn't exist, the callback will never be called.

  The callback MUST NOT MODIFY the state.

  This is a blocking operation that will prevent all other operations from executing until the
  callback returns. This operation is as expensive as your callback,
  so USE AS QUICKLY AND SPARINGLY AS POSSIBLE!!!

  See resolver.CurrentState for details.

  Safe for concurrent use.
*/
func (dc *DiegoCore) CurrentState(ns string, stateProcessor func(types.State)) {
  rs, ok := dc.nsManager.GetNamespace(ns)
  if !ok {
    return
  }
  rs.CurrentState(stateProcessor)
}
