#ifndef XINDB_UTIL_POSIXREAD_H
#define XINDB_UTIL_POSIXREAD_H

#include "Env.h"
#include "Status.h"

#include <unistd.h>

namespace xindb {

class PosixSequentialFile final : public SequentialFile {
 public:
    PosixSequentialFile(std::string filename, int fd)
        : fd_(fd), filename_(std::move(filename)) {}

    ~PosixSequentialFile() override { close(fd_); }

    Status Read(size_t n, Slice* result, char* scratch) override {
        Status status;
        while(true) {
            size_t read_size = read(fd_, scratch, n);
            if (read_size < 0) {        // Read error
                if (errno == EINTR) continue;
                status = PosixError(filename_, errno);
                break;
            }
            *result = Slice(scratch, read_size);
            break;
        }
        return status;
    }


 private:
    const int fd_;
    const std::string filename_;
};


}   // namespace xindb

#endif