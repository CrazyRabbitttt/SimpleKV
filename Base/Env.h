#ifndef XINDB_BASE_ENV_H
#define XINDB_BASE_ENV_H

#include "Status.h"

namespace xindb {


static Status PosixError(const std::string& context, int error_number) {
  if (error_number == ENOENT) {
    return Status::NotFound(context, strerror(error_number));
  } else {
    return Status::IOError(context, strerror(error_number));
  }
}

// 进行顺序写入文件的抽象， 需要有buffer防止频繁的写入小碎片
class WritableFile {
 public:
    WritableFile() = default;

    WritableFile(const WritableFile&) = delete;
    WritableFile& operator=(const WritableFile&) = delete;

    // 继承体系下析构函数需要是虚函数
    virtual ~WritableFile() = default;

    virtual Status Append(const Slice& data) = 0;
    virtual Status Close() = 0;
    virtual Status Flush() = 0;
    virtual Status Sync()  = 0;
 
};


class SequentialFile {
 public:
    SequentialFile() = default;

    SequentialFile(const SequentialFile&) = delete;
    SequentialFile& operator=(const SequentialFile&) = delete;

    virtual ~SequentialFile();

    virtual Status Read(size_t n, Slice* result, char* scratch) = 0;

    virtual Status Skip(uint64_t n) = 0;

};


}   // namespace xindb

#endif