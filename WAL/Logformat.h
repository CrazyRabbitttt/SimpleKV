#ifndef XINDB_WAL_LOGFORMAT_H
#define XINDB_WAL_LOGFORMAT_H

namespace xindb {
namespace Log {

enum RecordType {
    // 需要进行分段写入到磁盘空间中，BlockSize = 32KB
    kZeroType = 0,
    kFullType = 1,          // 一段占了全部的block

    // 一个block被分成不同的碎片🧩
    kFirstType = 2,
    kMiddleType= 3,
    kLastType  = 4,
};

static const int kMaxRecordType = kLastType;

static const int kBlockSize = 32 ;

// 直接不写crc checkSum 好了， Length(2 KB), RecordType(1 KB)
static const int kHeaderSize = 2 + 1;       


}   // namespace log
}   // namespace xindb

#endif