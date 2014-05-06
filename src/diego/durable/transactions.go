package durable

import "io"
import "os"
import "fmt"
import "math"
import "sync"
import "bufio"
import "bytes"
import "strings"
import "strconv"
import "encoding/binary"
import "encoding/gob"

import "diego/debug"
import "diego/resolver"

const dataExtension = ".txdata"
const indexExtension = ".txindex"
const defaultPerm = 0666

const maxChunkLength = 1000
const longLength = 8
const expectedIndexFileLength = maxChunkLength * longLength
const maxDataEntryLength = 16 * 1024

/*
TransactionLogger - durable write-ahead log for transactions
*/
type TransactionLogger struct {
  mu sync.Mutex

  // path where to store files
  // files are basePath + "0.txdata" / basePath + "0.txindex" etc.
  // no other files or directories may exist in this directory
  basePath string

  // last transaction id that was cleared
  clearedIndex int64

  // number of entries written in current chunk
  currentChunkEntryCount int64

  // number of bytes in current data chunk
  currentDataLength int64

  closed bool

  dataFiles map[int64]string
  indexFiles map[int64]string

  newestFileIndex int64
  newestDataFile *os.File
  newestIndexFile *os.File
}

// because Go's type system is lame (very long story)
// name needs to be short to keep gobs short
// gtr = gobbed transaction
type gtr struct {
  T resolver.Transaction
}

/*
CreateTransactionLogger - factory method for transaction writers
  see type description above for arg details
*/
func CreateTransactionLogger(basePath string) *TransactionLogger {
  tl := new(TransactionLogger)
  tl.basePath = basePath

  err := os.MkdirAll(basePath, defaultPerm)
  ensureNoError(err)

  tl.clearedIndex = -1
  tl.newestFileIndex = -1

  tl.dataFiles = make(map[int64]string)
  tl.indexFiles = make(map[int64]string)

  // check if there is already some data at the directory
  // if so, make it available to the transaction writer
  tryLoadExistingWriter(tl)

  return tl
}

/*
Append - durably appends the specified transaction to the log
  only returns after the log has been fsynced to disk
*/
func (tl *TransactionLogger) Append(t resolver.Transaction) {
  tl.mu.Lock()
  defer tl.mu.Unlock()

  if tl.closed {
    panic("Append called on a closed logger")
  }

  if tl.currentChunkEntryCount == maxChunkLength {
    tl.createNextFiles()
  }

  tl.currentChunkEntryCount++

  // write to the data file first
  // because it's easier to recover from a corrupt data file than from a corrupt index:
  //   simply look at what the last entry in the index is,
  //   and truncate the data file at that offset
  var buffer bytes.Buffer
  enc := gob.NewEncoder(&buffer)
  err := enc.Encode(gtr{t})
  ensureNoError(err)

  b := buffer.Bytes()
  if len(b) > maxDataEntryLength {
    panic(fmt.Sprintf("Transaction %v caused a data entry %d bytes long, longer than the max allowed", t, len(b)))
  }

  _, err = tl.newestDataFile.Write(b)
  ensureNoError(err)

  tl.currentDataLength += int64(len(b))

  // next, write the new entry to the index file
  binary.Write(tl.newestIndexFile, binary.LittleEndian, tl.currentDataLength)

  // finally, fsync on both files
  tl.newestDataFile.Sync()
  tl.newestIndexFile.Sync()
}

/*
Close - close the logger and do cleanup
  calling any function except Close on a closed logger will cause a panic
  Close is idempotent and can be called multiple times
*/
func (tl *TransactionLogger) Close() {
  tl.mu.Lock()
  defer tl.mu.Unlock()

  if tl.closed {
    return
  }

  tl.closed = true
  if tl.newestIndexFile != nil {
    err := tl.newestIndexFile.Close()
    ensureNoError(err)
    tl.newestIndexFile = nil
  }
  if tl.newestDataFile != nil {
    err := tl.newestDataFile.Close()
    ensureNoError(err)
    tl.newestDataFile = nil
  }
}

/*
ReadAll - reads all transactions in the entire log, in order,
  calling the callback function with every transaction

  VERY EXPENSIVE FUNCTION -- use only when recovering from faults
*/
func (tl *TransactionLogger) ReadAll(transactionProcessor func(resolver.Transaction)) {
  tl.mu.Lock()
  defer tl.mu.Unlock()

  if tl.closed {
    panic("ReadAll called on a closed logger")
  }

  var i int64
  for i = 0; i <= tl.newestFileIndex; i++ {
    tl.readAllFileIndex(i, transactionProcessor)
  }
}

// callers need to hold tl.mu before calling this method
func (tl *TransactionLogger) readAllFileIndex(index int64, transactionProcessor func(resolver.Transaction)) {
  // we're guaranteed that indexFiles and dataFiles have entries on the same keys
  // it's enough to check just one
  _, ok := tl.indexFiles[index]
  if !ok {
    return
  }

  indexFile, err := os.Open(tl.makePath(index, indexExtension))
  ensureNoError(err)

  dataFile, err := os.Open(tl.makePath(index, dataExtension))
  ensureNoError(err)

  fi, err := indexFile.Stat()
  ensureNoError(err)

  entryCount := fi.Size() / longLength

  indexReader := bufio.NewReader(indexFile)
  dataReader := bufio.NewReaderSize(dataFile, maxDataEntryLength)

  var dataItemEndOffset int64
  var dataItemStartOffset int64
  var i int64

  mainDataBytes := make([]byte, maxDataEntryLength)
  for i = 0; i < entryCount; i++ {
    dataItemStartOffset = dataItemEndOffset
    err = binary.Read(indexReader, binary.LittleEndian, &dataItemEndOffset)
    ensureNoError(err)

    length := dataItemEndOffset - dataItemStartOffset

    dataBytes := mainDataBytes[:length]
    io.ReadAtLeast(dataReader, dataBytes, int(length))

    buf := bytes.NewBuffer(dataBytes)

    dec := gob.NewDecoder(buf)
    var gobbed gtr
    err = dec.Decode(&gobbed)
    ensureNoError(err)

    transactionProcessor(gobbed.T)
  }

  err = indexFile.Close()
  ensureNoError(err)
  err = dataFile.Close()
  ensureNoError(err)
}

func (tl *TransactionLogger) assertValid() {
  tl.mu.Lock()
  defer tl.mu.Unlock()

  debug.Assert(tl.newestDataFile != nil,
               "Newest data file ptr is nil")
  debug.Assert(tl.newestIndexFile != nil,
               "Newest index file ptr is nil")

  fi, err := tl.newestDataFile.Stat()
  ensureNoError(err)

  debug.Assert(tl.currentDataLength == fi.Size(),
               "Expected data size %d but got %d", tl.currentDataLength, fi.Size())

  fi, err = tl.newestIndexFile.Stat()
  ensureNoError(err)

  entries := fi.Size() / longLength

  debug.Assert(tl.currentChunkEntryCount == entries,
               "Expected chunk entry count %d but got %d", tl.currentChunkEntryCount, entries)
  debug.Assert(tl.currentChunkEntryCount <= maxChunkLength,
               "Chunk entry count %d is greater than maxChunkLength", tl.currentChunkEntryCount)

  offset := tl.getFileEntryOffset(entries)
  debug.Assert(tl.currentDataLength == offset,
               "Expected data size %d but index shows %d", tl.currentDataLength, offset)
}

// callers need to hold tl.mu before calling this method
func (tl *TransactionLogger) getFileEntryOffset(i int64) int64 {
  if i < 0 || i > maxChunkLength {
    panic(fmt.Sprintf("Illegal file entry number %d", i))
  }

  if i == 0 {
    return 0
  }

  i--
  b := make([]byte, longLength)
  n, err := tl.newestIndexFile.ReadAt(b, i * longLength)
  ensureNoError(err)
  if n < longLength {
    panic(fmt.Sprintf("Couldn't read enough bytes for an int64, only read %d", n))
  }

  var offset int64
  buffer := bytes.NewBuffer(b)
  err = binary.Read(buffer, binary.LittleEndian, &offset)
  ensureNoError(err)

  return offset
}

// callers need to hold tl.mu before calling this method
// initializer is exempt from the above
func (tl *TransactionLogger) createNextFiles() {
  tl.newestFileIndex++
  tl.currentChunkEntryCount = 0
  tl.currentDataLength = 0
  var err error
  if tl.newestIndexFile != nil {
    err = tl.newestIndexFile.Close()
    ensureNoError(err)
  }
  if tl.newestDataFile != nil {
    err = tl.newestDataFile.Close()
    ensureNoError(err)
  }

  indexPath := tl.makePath(tl.newestFileIndex, indexExtension)
  dataPath := tl.makePath(tl.newestFileIndex, dataExtension)

  tl.indexFiles[tl.newestFileIndex] = indexPath
  tl.dataFiles[tl.newestFileIndex] = dataPath

  tl.newestIndexFile, err = os.OpenFile(indexPath,
                                        os.O_RDWR | os.O_CREATE,
                                        defaultPerm)
  ensureNoError(err)

  tl.newestDataFile, err = os.OpenFile(dataPath,
                                       os.O_RDWR | os.O_CREATE,
                                       defaultPerm)
  ensureNoError(err)
}

func ensureNoError(err error) {
  if err != nil {
    panic(err)
  }
}

func (tl *TransactionLogger) makePath(index int64, ext string) string {
  return fmt.Sprintf("%s%c%d%s", tl.basePath, os.PathSeparator, index, ext)
}

func tryLoadExistingWriter(tl *TransactionLogger) {
  dir, err := os.Open(tl.basePath)
  ensureNoError(err)

  names, err := dir.Readdirnames(0)
  ensureNoError(err)

  err = dir.Close()
  ensureNoError(err)

  if len(names) == 0 {
    tl.createNextFiles()
    return
  }

  // verify file names and memoize data/index files
  for _, n := range names {
    sp := strings.SplitN(n, ".", 2)
    if len(sp) != 2 {
      panic(fmt.Sprintf("Unexpected file %s found at basePath %s", n, tl.basePath))
    }

    index, err := strconv.ParseInt(sp[0], 0, 64)
    ensureNoError(err)

    if index < 0 {
      panic(fmt.Sprintf("Unexpected file index %s found at basePath %s", n, tl.basePath))
    }

    sp[1] = "." + sp[1]
    switch sp[1] {
    case dataExtension:
      tl.dataFiles[index] = n
    case indexExtension:
      tl.indexFiles[index] = n
    default:
      panic(fmt.Sprintf("Unexpected file extension of file %s found at basePath %s", n, tl.basePath))
    }
  }

  // ensure all data files have matching indices
  for k := range tl.dataFiles {
    _, ok := tl.indexFiles[k]
    if !ok {
      panic(fmt.Sprintf("Found data file %d but not its index file, at basePath %s", k, tl.basePath))
    }
  }

  // ensure all index files have matching data files
  for k := range tl.indexFiles {
    _, ok := tl.dataFiles[k]
    if !ok {
      panic(fmt.Sprintf("Found index file %d but not its data file, at basePath %s", k, tl.basePath))
    }
  }

  minFileIndex := int64(math.MaxInt64)
  maxFileIndex := int64(-1)
  for k := range tl.dataFiles {
    if minFileIndex > k {
      minFileIndex = k
    }
    if maxFileIndex < k {
      maxFileIndex = k
    }
  }

  tl.clearedIndex = (minFileIndex * maxChunkLength) - 1

  tl.newestFileIndex = maxFileIndex
  tl.newestIndexFile, err = os.OpenFile(tl.makePath(maxFileIndex, indexExtension),
                                        os.O_RDWR | os.O_APPEND,
                                        defaultPerm)
  ensureNoError(err)

  fi, err := tl.newestIndexFile.Stat()
  ensureNoError(err)

  // if the newest index file is full, make new files
  if fi.Size() == expectedIndexFileLength {
    tl.createNextFiles()
    return
  }

  // ensure there are an integer number of entries in the index
  if fi.Size() % longLength != 0 {
    panic(fmt.Sprintf("Illegal index file size %d, non-integer number of entries", fi.Size()))
  }

  // calculate the expected length of the data file
  if fi.Size() == 0 {
    tl.currentChunkEntryCount = 0
    tl.currentDataLength = 0
  } else {
    tl.currentChunkEntryCount = fi.Size() / longLength
    tl.currentDataLength = tl.getFileEntryOffset(tl.currentChunkEntryCount)
  }

  // prep the newest index's corresponding data file
  tl.newestDataFile, err = os.OpenFile(tl.makePath(maxFileIndex, dataExtension),
                                       os.O_RDWR | os.O_APPEND,
                                       defaultPerm)
  fi, err = tl.newestDataFile.Stat()
  ensureNoError(err)

  // verify the data file's length
  if fi.Size() != tl.currentDataLength {
    panic(fmt.Sprintf("Expected %d bytes in data file, found %d", tl.currentDataLength, fi.Size()))
  }
}
