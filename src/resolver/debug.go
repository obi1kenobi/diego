package resolver

import "fmt"

const debugLevel = 0
const maxStringifyLength = 500

/*
Stringify - return the text representation of the value, or placeholder if too long
*/
func Stringify(value interface{}) string {
  debugv := fmt.Sprintf("%v", value)
  if len(debugv) > maxStringifyLength {
    debugv = "<too long>"
  }
  return debugv
}

/*
DPrintf - tiered logging
*/
func DPrintf(level int, format string, a ...interface{}) (n int, err error) {
  if level <= debugLevel {
    n, err = fmt.Printf(format + "\n", a...)
  }
  return
}

/*
Assert - if condition is false, panic with specified message
*/
func Assert(cond bool, str string) {
  if !cond {
    panic(str)
  }
}
