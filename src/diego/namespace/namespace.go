package namespace

import "sync"
import "diego/resolver"

/*
NamespaceManager - Manages access, creation and deletion of namespaces
*/
type NamespaceManager struct {
  mu sync.RWMutex
  namespaces map[string]*resolver.Resolver
}

/*
CreateNamespaceManager - Factory method for NamespaceManager. Initializes the namespaces map
*/
func CreateNamespaceManager() *NamespaceManager {
  ns := new(NamespaceManager)
  ns.namespaces = make(map[string]*resolver.Resolver)
  return ns
}

/*
CreateNamespace - Make a named namespace from a name and a resolver. Returns true if a
  new namespace was created, and false if one with that name already existed. Also returns the resolver for
  the namespace of that name (may not be the same resolver that was passed in).
*/
func (ns *NamespaceManager) CreateNamespace(name string, resolver *resolver.Resolver) bool {
  ns.mu.Lock()
  defer ns.mu.Unlock()

  _, ok := ns.namespaces[name]
  if ok {
    return false
  }

  ns.namespaces[name] = resolver
  return true
}

/*
GetNamespace - Get the resolver for a namespace
*/
func (ns *NamespaceManager) GetNamespace(name string) (*resolver.Resolver, bool) {
  ns.mu.RLock()
  defer ns.mu.RUnlock()

  resolver, ok := ns.namespaces[name]
  return resolver, ok
}

/*
RemoveNamespace - Removes the specified namespace, if it exists
*/
func (ns *NamespaceManager) RemoveNamespace(name string) bool {
  ns.mu.Lock()
  defer ns.mu.Unlock()

  resolver, ok := ns.namespaces[name]
  if !ok {
    return false
  }

  resolver.Close()
  delete(ns.namespaces, name)
  return true
}
