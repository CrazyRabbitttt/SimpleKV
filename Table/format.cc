
#include "format.h"
#include "Coding.h"
#include "PosixEnv.h"
#include "CrcChecksum.h"
#include "Options.h"

namespace xindb {

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
    printf("magic_ptr : [%s]\n", magic_ptr);
    uint32_t Magic_Number_Low = DecodeFixed32(magic_ptr);
    uint32_t Magic_Number_Hig = DecodeFixed32(magic_ptr + 4);
    uint64_t Magic_Number = ((static_cast<uint64_t>(Magic_Number_Hig) << 32) |
                              (static_cast<uint64_t>(Magic_Number_Low))); 
    printf("Magic number decoded : %lld, kMagicnum : %lld\n", (uint64_t)(Magic_Number), (uint64_t)(kTableMagicNumber));   

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


// ReadBlock(file, opt, footer.index_handle(), &index_block_contents);

// 从文件中读取 block 出来[通过blockhandle]，暂存到 blockcontents 中
Status ReadBlock(RandomAccessFile* file, const ReadOptions& options, 
                const BlockHandle& handle, BlockContents* result) {

    result->data = Slice();
    result->cacheable = false;
    result->heap_allocated = false;         // not alloc by heap 

    size_t n = static_cast<size_t>(handle.size());
    char* buf = new char[n + kBlockTrailerSize];
    Slice contents; 
    // 从offset的位置读取n字节数据写到 scratch 中, 加上最后的 5B[type|crc32]
    Status s = file->Read(handle.offset(), n + kBlockTrailerSize, &contents, buf);
    if (!s.ok()) {
        delete[] buf;
        return s;
    }
    
    if (contents.size() != n + kBlockTrailerSize) {
        delete[] buf;
        return Status::Corruption("truncated block read");
    }

    // 检查crc校验和， 判断读取到的block是否是正确的
    const char* data = contents.data();
    if (options.verify_checksums) {
        // 拿到crc校验和与类型
        const uint32_t crc = tinycrc::crc32(data, n);       // type 没有加到 crc 里面
        const uint32_t actual = DecodeFixed32(data + n + 1);
        if (crc != actual) {
            delete[] buf;
            s = Status::Corruption("block checksum mismatch");
            return s;
        } else {
            printf("Read Block : crc check sum right!\n");
        }
    }

    switch (data[n]) {
        case kNoCompression:
            if (data != buf) {
                delete[] buf;
                result->data = Slice(data, n);
                result->heap_allocated = false;
                result->cacheable = false;
            } else {
                result->data = Slice(buf, n);
                result->heap_allocated = true;
                result->cacheable = true;
            }
            break;
        case kSnappyCompress :
            if (data != buf) {
                delete[] buf;
                result->data = Slice(data, n);
                result->heap_allocated = false;
                result->cacheable = false;
            } else {
                result->data = Slice(buf, n);
                result->heap_allocated = true;
                result->cacheable = true;
            }
            break;
        default:
            delete[] buf;
            return Status::Corruption("bad block type");
    }

    return Status::OK();
}

}   // namespace xindb
