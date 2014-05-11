package textot

import "testing"
import "strings"
import "strconv"
import "diego/tests"
import "diego/resolver"
import "diego/types"

const trailingDistance = 50

func setup()(*resolver.Resolver, *textState, func() types.RequestToken) {
  return makeResolver(), makeState().(*textState), types.MakeRequestTokenGenerator(int64(0))
}

func makeResolver()*resolver.Resolver {
  return resolver.CreateResolver(makeState, trailingDistance, "")
}

func stateEquals(a, b types.State)bool {
  txt := a.(*textState)
  txt2 := b.(*textState)
  return txt.Str == txt2.Str && txt.Tid == txt2.Tid
}

func textPredicate(start, finish int, str string) func(*testing.T, types.State)bool {
  return func(t *testing.T, s types.State)bool {
    txt := s.(*textState).Str
    if finish == -1 {
      finish = len(txt)
    }
    if start > finish || finish > len(txt) {
      t.Errorf("Slice %d:%d invalid for txt of length %d", start, finish, len(txt))
      return false
    }
    if txt[start:finish] != str {
      t.Errorf("Expected '%s', but got '%s' instead.", str, txt[start:finish])
      return false
    }
    return true
  }
}

func parseTextOpToken(token string) textOp {
  switch {
  case token[0] == "{"[0]:
    num, _ := strconv.ParseInt(token[1:len(token)-1], 10, 32)
    return textOp{OpType: textDeleteOp, N: int(num)}
  case token[0] == "'"[0]:
    return textOp{OpType: textInsertOp, Str: token[1:len(token)-1]}
  default:
    num, _ := strconv.ParseInt(token, 10, 32)
    return textOp{OpType: textSkipOp, N: int(num)}
  }
}

func makeTransaction(id int64, token types.RequestToken, str string) *textTransform {
  tokens := strings.Split(str, ", ")
  ops := make([]textOp, len(tokens))
  for i, token := range tokens {
    ops[i] = parseTextOpToken(token)
  }
  transform := &textTransform{}
  transform.Tid = id
  transform.Token = token
  transform.Ops = ops
  return transform
}

func TestAtMostOnce(t *testing.T) {
  rs, s, nt := setup()

  token1 := nt()
  token2 := nt()
  token3 := nt()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(makeTransaction(0, token1, "'aaa'"), tests.Success, textPredicate(0, -1,"aaa")),
    tests.MakeTestDataItem(makeTransaction(1, token2, "'b'"), tests.SuccessLost, textPredicate(0, -1, "baaa")),
    tests.MakeTestDataItem(makeTransaction(1, token2, "'b'"), tests.Success, textPredicate(0, -1, "baaa")),
    tests.MakeTestDataItem(makeTransaction(2, token3, "2, 'b'"), tests.SuccessLost, textPredicate(0, -1, "babaa")),
    tests.MakeTestDataItem(makeTransaction(2, token3, "2, 'b'"), tests.Success, textPredicate(0, -1, "babaa")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "1, 'cc'"), tests.Success, textPredicate(0, -1, "babccaa")),
    tests.MakeTestDataItem(makeTransaction(4, nt(), "1, 'd', 3, 'dd'"), tests.Success, textPredicate(0, -1, "bdabcddcaa")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestSkipOp(t *testing.T) {
  rs, s, nt := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(makeTransaction(0, nt(), "'aaa'"), tests.Success, textPredicate(0, -1,"aaa")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "1"), tests.Success, textPredicate(0, -1,"aaa")),
    tests.MakeTestDataItem(makeTransaction(2, nt(), "1, 1"), tests.Success, textPredicate(0, -1, "aaa")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "2"), tests.Success, textPredicate(0, -1, "aaa")),
    tests.MakeTestDataItem(makeTransaction(4, nt(), "1"), tests.Success, textPredicate(0, -1, "aaa")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestInsertOp(t *testing.T) {
  rs, s, nt := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(makeTransaction(0, nt(), "'aaa'"), tests.Success, textPredicate(0, -1,"aaa")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "'b'"), tests.Success, textPredicate(0, -1, "baaa")),
    tests.MakeTestDataItem(makeTransaction(2, nt(), "2, 'b'"), tests.Success, textPredicate(0, -1, "babaa")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "1, 'cc'"), tests.Success, textPredicate(0, -1, "babccaa")),
    tests.MakeTestDataItem(makeTransaction(4, nt(), "1, 'd', 3, 'dd'"), tests.Success, textPredicate(0, -1, "bdabcddcaa")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestDeleteOp(t *testing.T) {
  rs, s, nt := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(makeTransaction(0, nt(), "'aaabbbcccddd'"), tests.Success, textPredicate(0, -1,"aaabbbcccddd")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "{1}"), tests.Success, textPredicate(0, -1, "aabbbcccddd")),
    tests.MakeTestDataItem(makeTransaction(2, nt(), "2, {2}"), tests.Success, textPredicate(0, -1, "aabcccddd")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "1, {1}"), tests.Success, textPredicate(0, -1, "abcccddd")),
    tests.MakeTestDataItem(makeTransaction(4, nt(), "1, {2}, 3, {1}"), tests.Success, textPredicate(0, -1, "accdd")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}

func TestMixedResolveOp(t *testing.T) {
  rs, s, nt := setup()

  testData := []tests.TestDataItem {
    tests.MakeTestDataItem(makeTransaction(0, nt(), "'aaabbb'"), tests.Success, textPredicate(0, -1,"aaabbb")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "1, 'cc', 1, {1}"), tests.Success, textPredicate(0, -1, "accabbb")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "5, 'ddd'"), tests.Success, textPredicate(0, -1, "accabbdddb")),
    tests.MakeTestDataItem(makeTransaction(1, nt(), "1, {5}"), tests.Success, textPredicate(0, -1, "accddd")),
    tests.MakeTestDataItem(makeTransaction(3, nt(), "3, 'ee', 1, 'f'"), tests.Success, textPredicate(0, -1, "acceefddd")),
  }

  tests.RunSequentialTest(t, rs, testData, s, stateEquals)
}
