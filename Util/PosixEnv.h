#ifndef XINDB_UTIL_POSIXENV_H_
#define XINDB_UTIL_POSIXENV_H_

#include "Env.h"

#include "unistd.h"
#include <iostream>
#include <atomic>


namespace xindb {

class Limiter {

 public:
    Limiter(int max_acquires) : acquires_allowed_(max_acquires) {}

    Limiter(const Limiter&) = delete;
    Limiter operator=(const Limiter&) = delete;

    bool Acquire() {
        int old_acquires_allowed =
        acquires_allowed_.fetch_sub(1, std::memory_order_relaxed);

        if (old_acquires_allowed > 0) return true;

        acquires_allowed_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    // 没大看懂
    void Release() { acquires_allowed_.fetch_add(1, std::memory_order_relaxed); }

 private:

    std::atomic<int> acquires_allowed_;     // 最大的允许获取的资源，防止访问过多而崩溃

};



class PosixRandomAccessFile : public RandomAccessFile{

 public:
    PosixRandomAccessFile(std::string filename, int fd)
        : fd_(fd), filename_(std::move(filename)) {}

    ~PosixRandomAccessFile() override {
       ::close(fd_);  
    }

    // 具体的 Read 
    Status Read(uint64_t offset, size_t n, Slice* result, char* scratch) const override {
        int fd = fd_;   
        printf("filename : %s\n", filename_.c_str());
        Status status;
        // 从offset的位置读取n字节数据写到 scratch 中
        printf("Read from file, the offset : %d\n", offset);
        ssize_t read_size = ::pread(fd, scratch, n, static_cast<off_t>(offset));
        *result = Slice(scratch, (read_size < 0) ? 0 : read_size);
        printf("Read result from file :[%s], len : %d\n", (char*)result->data(), result->size());
        if (read_size < 0) {
            status = PosixError(filename_, errno);
        }
        return status;
    }


 private:
    const int fd_;
    const std::string filename_;

};  

}   // namespace xindb

#endif