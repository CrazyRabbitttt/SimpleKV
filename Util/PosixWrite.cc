#include "PosixWrite.h"

#include <string.h>

using namespace xindb;

Status PosixWritableFile::Append(const Slice& data)  {
    // 写到buffer中
    size_t write_size = data.size();
    const char* write_data = data.data();

    // 需要写的size 和 剩余的空间
    size_t copy_size = std::min(write_size, kWritableFilebufferSize - pos_);
    memcpy(buf_ + pos_, write_data, copy_size);

    write_data += copy_size; write_size -= copy_size;
    pos_ += copy_size;          // 忘了这一步了

    // 如果说都写到buffer 中去了
    if (write_size == 0) {
        return Status::OK();
    }

    // 填到buffer中还是不行，只能是先刷盘
    Status status = FlushBuffer();
    if (!status.ok()) return status;

    // 到这进行判断了：1.数据量小走buffer 2。数据量大直接写
    if (write_size < kWritableFilebufferSize) {
        memcpy(buf_, write_data, write_size);
        pos_ = write_size;
        return Status::OK();
    }

    // 这里就是生于要写的大雨buffer
    return WriteUnbuffered(write_data, write_size);

}

Status PosixWritableFile::Close() {
    Status status  = FlushBuffer();      // 首先进行刷盘
    int is_closed = ::close(fd_);
    if (is_closed < 0 && status.ok()) {
        fd_ = -1;
        return Status::IOError(filename_, "close error");
    }
    fd_ = -1;
    return status;
}


Status PosixWritableFile::Flush()  {
    return FlushBuffer();
}



Status PosixWritableFile::Sync() {
    // do nothing, haha
    return Status::OK();
}

