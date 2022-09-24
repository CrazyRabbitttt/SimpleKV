#ifndef XINDB_UTIL_POSIXWRITE_H
#define XINDB_UTIL_POSIXWRITE_H

#include "Env.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

namespace xindb {


// 写文件首先写到buffer中进行顺序的写入
const size_t kWritableFilebufferSize = 65535;   


class PosixWritableFile final : public WritableFile{
 public:
    PosixWritableFile(std::string filename, int fd)
        :pos_(0),
         fd_(fd),
         filename_(std::move(filename)),
         dirname_(Dirname(filename)) {}
    
   ~PosixWritableFile() override {
        if (fd_ >= 0) {
            Close();            // close the file
        }
   }

    Status Append(const Slice& data) override;
    Status Close() override;
    Status Flush() override;
    Status Sync() override;


 private:

    // 将Buffer中的数据刷盘
    Status FlushBuffer() {
        Status status = WriteUnbuffered(buf_, pos_);
        pos_ = 0;
        return status;
    }

    // 直接写到到磁盘上面
    Status WriteUnbuffered(const char* data, size_t size) {
        while (size > 0) {
            ssize_t write_size = ::write(fd_, data, size);
            if (write_size < 0) return Status::IOError(filename_, "errno");

            data += write_size;
            size -= write_size;
        }
        return Status::OK();
    }

    // 获得绝对路径 Or 相对路径的文件的dirname
    static std::string Dirname(const std::string& filename) {
        std::string::size_type index = filename.rfind('/');
        if (index == std::string::npos) {
            // 压根就没有'/'
            return std::string(".");
        }

        return filename.substr(0, index);
    }

    char buf_[kWritableFilebufferSize];
    size_t pos_;            // buf已经使用了的位置
    int fd_;

    const std::string filename_;
    const std::string dirname_;
};

}   // namespace xindb
#endif