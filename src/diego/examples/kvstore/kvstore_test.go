package kvstore

import "testing"
import "diego/tests"

func TestLwwSet(t *testing.T) {
  rs, s := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(&lwwSetOp{0, "a", "b"}, tests.Success, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&lwwSetOp{0, "a", "c"}, tests.Success, keyValuePredicate("a", "c")),
    tests.MakeTestDataItem(&lwwSetOp{0, "a", "abcd"}, tests.Success, keyValuePredicate("a", "abcd")),
    tests.MakeTestDataItem(&lwwSetOp{3, "a", "def"}, tests.Success, keyValuePredicate("a", "def")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestOptimisticSet(t *testing.T) {
  rs, s := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(&pessimisticSetOp{0, "a", "b"}, tests.Success, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&pessimisticSetOp{0, "a", "c"}, tests.Failure, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&pessimisticSetOp{1, "a", "c"}, tests.Success, keyValuePredicate("a", "c")),
    tests.MakeTestDataItem(&pessimisticSetOp{2, "a", "abcd"}, tests.Success, keyValuePredicate("a", "abcd")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestTestAndSet(t *testing.T) {
  rs, s := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(&testAndSetOp{0, "a", "b"}, tests.Success, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&testAndSetOp{0, "a", "c"}, tests.Failure, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&testAndSetOp{1, "a", "c"}, tests.Success, keyValuePredicate("a", "c")),
    tests.MakeTestDataItem(&lwwSetOp{0, "a", "d"}, tests.Success, keyValuePredicate("a", "d")),
    tests.MakeTestDataItem(&testAndSetOp{1, "a", "c"}, tests.Failure, keyValuePredicate("a", "d")),
    tests.MakeTestDataItem(&appendOp{0, "a", "c"}, tests.Success, keyValuePredicate("a", "dc")),
    tests.MakeTestDataItem(&appendOp{0, "a", "e"}, tests.Success, keyValuePredicate("a", "dce")),
    tests.MakeTestDataItem(&testAndSetOp{4, "a", "e"}, tests.Failure, keyValuePredicate("a", "dce")),
    tests.MakeTestDataItem(&testAndSetOp{5, "a", "ef"}, tests.Success, keyValuePredicate("a", "ef")),
    tests.MakeTestDataItem(&testAndSetOp{0, "b", "ab"}, tests.Success, keyValuePredicate("b", "ab")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestAppend(t *testing.T) {
  rs, s := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(&appendOp{0, "a", "b"}, tests.Success, keyValuePredicate("a", "b")),
    tests.MakeTestDataItem(&appendOp{0, "a", "c"}, tests.Success, keyValuePredicate("a", "bc")),
    tests.MakeTestDataItem(&appendOp{1, "a", "d"}, tests.Success, keyValuePredicate("a", "bcd")),
    tests.MakeTestDataItem(&appendOp{3, "a", "ef"}, tests.Success, keyValuePredicate("a", "bcdef")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestFlipflopAdd(t *testing.T) {
  rs, s := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(&flipflopAddOp{0, "a", 0, 2}, tests.Success, keyValuePredicate("a", "2")),
    tests.MakeTestDataItem(&flipflopAddOp{1, "a", 1, 1}, tests.Success, keyValuePredicate("a", "1")),
    tests.MakeTestDataItem(&flipflopAddOp{1, "a", 1, 3}, tests.Success, keyValuePredicate("a", "4")),
    tests.MakeTestDataItem(&flipflopAddOp{0, "a", 0, 4}, tests.Success, keyValuePredicate("a", "0")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestRandomized(t *testing.T) {
  tests.RunRandomizedSequentialTests(t, makeResolver, makeState, makeTestData, stateEquals)
}
