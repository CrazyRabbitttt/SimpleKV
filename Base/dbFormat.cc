#include "dbFormat.h"
#include "Slice.h"
#include "Coding.h"

using namespace xindb;

// 将seq 和 type 打包📦
static uint64_t PackSeqAndType(uint64_t seq, ValueType type) {
    return (seq << 8) | type; 
}


LookUpKey::LookUpKey(const Slice& user_key, SequencrNumber sequence) {
    size_t usize = user_key.size();
    size_t needed = usize + 13;     // 保守的估计

    char* dst;
    if (needed <= sizeof(space_)) {
        dst = space_;
    } else {
        dst = new char[needed];
    }

    //   | klen[varint32] | UserKey[char*] | Tag[8 bytes]  |
    // start             kstart                  end 

    start_ = dst;
    dst = EncodeVarint32(dst, usize + 8);
    kstart_ = dst;                              // 压缩进dst之后指针的位置会改变的

    memcpy(dst, user_key.data(), usize);
    dst += usize;

    //                                  seq,    默认：value类型
    EncodeFixed64(dst, PackSeqAndType(sequence, kValueTypeForSeek));    
    dst += 8;
    end_ = dst;
}



