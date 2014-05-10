package namespace

import "diego/resolver"
import "diego/debug"
import "diego/types"

/*
AssertNamespacesEqual - ensure that two namespace managers have the same contents.
Useful for durability checks
*/
func AssertNamespacesEqual (ns1, ns2 *NamespaceManager,
                            stateEquals func(a, b types.State)bool,
                            transactionEquals func(a, b types.Transaction)bool) {
  debug.Assert(ns1 != nil, "namespace 1 did not exist")
  debug.Assert(ns2 != nil, "namespace 1 did not exist")

  ns1.mu.Lock()
  defer ns1.mu.Unlock()
  ns2.mu.Lock()
  defer ns2.mu.Unlock()

  numSpaces := 0
  for name, space := range ns1.namespaces {
    numSpaces++
    resolver.AssertResolversEqual(space, ns2.namespaces[name], stateEquals, transactionEquals)
  }

  debug.Assert(len(ns2.namespaces) == numSpaces,
      "incorrect number of namespaces, %d != %d.", len(ns2.namespaces), numSpaces)
}

/*
CloseAllNamespaces - close all existing namespaces.
*/
func CloseAllNamespaces (ns *NamespaceManager) {
  ns.mu.Lock()
  defer ns.mu.Unlock()

  for _, space := range ns.namespaces {
    space.Close()
  }
}
