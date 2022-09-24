#include "Log_writer.h"
#include "Coding.h"

using namespace xindb;
using namespace Log;

Writer::Writer(PosixWritableFile* dest)
    : dest_(dest), block_offset_(0) {}


Writer::Writer(PosixWritableFile* dest, uint64_t dest_len) 
    : dest_(dest), block_offset_(dest_len % kBlockSize) {}

Writer::~Writer() = default;

Status Writer::AddRecord(const Slice& slice) {
    size_t left = slice.size();
    const char* ptr = slice.data();

    Status status;
    bool begin = true;      // 是不是begin 
    do {
        int avil = kBlockSize - block_offset_;
        assert(avil >= 0);
        if (avil < kHeaderSize) {       // 连盛下头部的空间都没了
            if (avil > 0) {
                dest_->Append(Slice("\x00\x00\x00", avil)); // 填充头部3B
            }
            block_offset_ = 0;
        }

        const size_t avail = kBlockSize - block_offset_ - kHeaderSize;
        const size_t fragment_len = (left < avail) ? left : avail;
        RecordType type;
        const bool end = (left == fragment_len);        // 剩下的能够填充
        if (begin && end) {
            type = kFullType;
        } else if (begin) {
            type = kFirstType;
        } else if (end) {   
            type = kLastType;
        } else {
            type = kMiddleType;
        }

        status = EmitPhysicalRecord(type, ptr, fragment_len);
        ptr  += fragment_len;
        left -= fragment_len;
        begin = false;
    } while (status.ok() && left > 0);
    return status;
}

Status Writer::EmitPhysicalRecord(RecordType type, const char* ptr, size_t len) {

    // len(2 Bytes), Type(1Bytes)
    char buf[kHeaderSize];
    buf[0] = static_cast<char>(len & 0xff);
    buf[1] = static_cast<char>(len >> 8);
    buf[2] = static_cast<char>(type);

    // 写进Log中，写完刷盘
    Status status = dest_->Append(Slice(buf, kHeaderSize));
    if (status.ok()) {
        status = dest_->Append(Slice(ptr, len));
        if (status.ok()) {
            status = dest_->Flush();
        }
    }

    block_offset_ += len + kHeaderSize;
    return status;
}

