package lego

// Transactions
type LegoTransaction struct {
  id int64
  ops []interface{}
}

func (xa *LegoTransaction) SetId(id int64) {
  xa.id = id
}

func (xa *LegoTransaction) Id() int64 {
  return xa.id
}
