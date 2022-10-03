
#include "format.h"
#include "Coding.h"

using namespace xindb;


void BlockHandle::EncodeTo(std::string *dst) const {
    PutVarint64(dst, offset_);      // 内部调用的是append
    PutVarint64(dst, size_);
}


Status BlockHandle::DecodeFrom(Slice* input) {
    if (GetVarint64(input, &offset_) && GetVarint64(input, &size_)) {
        // get之后内部会更改一下input,移动一下齐起始的位置
        return Status::OK();
    } else {
        return Status::Corruption("bad block handle");
    }
}

// 将footer写进dst中
void Footer::EncodeTo(std::string* dst) const {

    const int originsize = dst->size();     // 一直没用过， 编译报 warning 
    // 1.将index[s] 放进去
    metaIndex_handle_.EncodeTo(dst);
    index_handle_.EncodeTo(dst);

    // 2.Padding
    dst->resize(2 * BlockHandle::kMaxEncodedLength);    

    // 3.MagicNumber
    PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber & 0xffffffffu));
    PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber >> 32));
    assert(originsize + kEncodedLen == dst->size());
    (void)originsize;                      // 清除掉编译时Warning 
}

// 解析foot, Fixed Size, 能够通过 Size 直接 读取到数据
Status Footer::DecodeFrom(Slice* input) {
    // 1.我们首先解析魔数
    const char* magic_ptr = input->data() + kEncodedLen - 8;      // 魔数的起始位置
    uint32_t Magic_Number_Low = DecodeFixed32(magic_ptr);
    uint32_t Magic_Number_Hig = DecodeFixed32(magic_ptr + 4);
    uint64_t Magic_Number = static_cast<uint64_t>(Magic_Number_Hig << 32) | static_cast<uint64_t>(Magic_Number_Low);    

    if (Magic_Number != kTableMagicNumber) {
        return Status::Corruption("not an sstable(bad magic numer)");
    }

    // 2.解析metahandle & indexhandle
    Status result = metaIndex_handle_.DecodeFrom(input);
    if (result.ok()) {
        result = index_handle_.DecodeFrom(input);
    }
    // 3.去除掉padding
    if (result.ok()) {
        const char* end = magic_ptr + 8;
        *input = Slice(end, input->data() + input->size() - end);
    }
    return result;
}


