
#include "Status.h"

#include <string.h>
#include <string>


namespace xindb {

//进行状态的拷贝用于Status的拷贝行为,将最终的state进行返回
const char* Status::CopyState(const char* state) {
    //为什么不直接进行state的赋值呢？沈拷贝问题吗？
    uint32_t size;
    memcpy(&size, state, sizeof(size));    
    char* result = new char[size + 5];
    memcpy(result, state, size + 5);
    return result;
}




//进行Status的实现，Code, message 
Status::Status(Code code, const Slice& msg, const Slice& msg2) {
    //前四位是len 1字节Code 剩下的是message
    assert(code != kOK);
    const uint32_t len1 = static_cast<uint32_t>(msg.size());
    const uint32_t len2 = static_cast<uint32_t>(msg2.size());
    const uint32_t size = len1 + (len2 ? (2 + len2) : 0);       //如果有Silce2的话，加上 ： 
    char* result = new char[size + 5];
    //没错，就是将size的内容写到result上面 ,first 4bytes
    memcpy(result, &size, sizeof(size));                   
    //the 5th byte : code
    result[4] = static_cast<char>(code);
    memcpy(result + 5, msg.data(), len1);

    if (len2) {
        //如果len2不是空的话，用": "进行连接
        result[5 + len1] = ':';
        result[6 + len1] = ' ';
        memcpy(result + 7 + len1, msg2.data(), len2);
    }
    state_ = result;
}


//将具体的状态信息积进行替换成为String的形式
std::string Status::ToString() const {
    if (state_ == nullptr) {
        return "OK";
    } else {
        char tmp[30];
        const char* type;
        switch(code()) {
            case kOK:
                type = "OK";
                break;
            case kNotFound:
                type = "NotFound: ";
                break;
            case kCorruption:
                type = "Corruption: ";
                break;
            case kNotSupported:
                type = "Not implement: ";
                break;
            case kInvalidArgument:
                type = "Invalid argument: ";
                break;
            case kIOError:
                type = "IO error: ";
                break;
            default: 
              std::snprintf(tmp, sizeof(tmp), "Unknown code(%d):", static_cast<int>(code()));
              type = tmp;
              break;
        }

        //下面将type & state进行拼接
        std::string result(type);

        uint32_t length;
        //从state中提取出len，前四个字节
        memcpy(&length, state_, sizeof(length));

        result.append(state_ + 5, length);
        return result;

    }
}

}   // namespace xindb 
