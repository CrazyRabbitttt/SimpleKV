#ifndef XIN_UTIL_STATUS_H
#define XIN_UTIL_STATUS_H

#include "Slice.h"

#include <string.h>

namespace xindb {
class Status {

public:
//create the status of success 
    Status() noexcept : state_(nullptr) {}

    ~Status() {delete[] state_;}

    //默认的拷贝行为
    Status(const Status& rhs);
    Status& operator=(const Status& rhs);

    // Status(Status&& rhs) noexcept: state_(rhs.state_) {rhs.state_ = nullptr;}
    Status& operator=(Status& rhs) noexcept;


    //Return a success status.
    static Status OK() {return Status();}

    //Return error status of type 
    static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(kNotFound, msg, msg2);
    }
    static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(kCorruption, msg, msg2);
    }
    static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(kNotSupported, msg, msg2);
    }
    static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(kInvalidArgument, msg, msg2);
    }
    static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(kIOError, msg, msg2);
    }

    //用来判断状态是否是指定的状态
    bool ok() const {return (state_ == nullptr);}

    bool IsNotFound() const {return code() == kNotFound;}

    bool IsCorruption() const {return code() == kCorruption;}

    bool IsIOError() const {return code() == kIOError;}

    bool IsNotSupported() const {return code() == kNotSupported;}

    bool IsInvalidArgument() const {return code() == kInvalidArgument;}

    //Return a string representation the status 
    std::string ToString() const;


private:
    enum Code {
        kOK = 0,
        kNotFound = 1,
        kCorruption = 2, 
        kNotSupported = 3,
        kInvalidArgument = 4,
        kIOError = 5
    };

    Code code() const {
        return (state_ == nullptr) ? kOK : static_cast<Code>(state_[4]);
    }


    Status(Code code, const Slice& msg, const Slice& msg2);

    static const char* CopyState(const char* s);


    //如果状态是OK的话，那么state就是空的
    /*
        state[0, 3] = len of message
        state[4]    = code
        state[5]    = message 
    */
    const char* state_;
};


inline Status::Status(const Status& rhs) {
    //这里应该是防止出现沈拷贝问题，而不是直接进行赋值
    state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
}

inline Status& Status::operator=(const Status& rhs) {
    //case1: this == &rhs
    //case2: *this and rhs == ok
    if (state_ != rhs.state_) {
        delete[] state_;
        state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
    }
    //如果都是Ok的话
    return *this;
}

}   // namespace xindb

#endif