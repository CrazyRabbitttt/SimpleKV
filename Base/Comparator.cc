#include "Comparator.h"
#include "Slice.h"

namespace xindb {

// 找到(start, limit) 之间的比较短的字符串赋值给 start，用于标识 DataBlock 的边界
void ByteWiseComparator::FindShortestSeparator(std::string* start, const Slice& limit) const {
    int min_len = std::min(start->size(), limit.size());
    int commom_pre = 0;
    // 获得前缀长度
    while (commom_pre < min_len && (*start)[commom_pre] == limit[commom_pre]) commom_pre++;
    if (commom_pre == min_len) {
        // 某个字符串是另一个的子字符串，
    } else {
        uint8_t diff_byte = static_cast<uint8_t>((*start)[commom_pre]);         // 不同的那个char
        if (diff_byte < static_cast<uint8_t>(0xff) && diff_byte + 1 < static_cast<uint8_t>(limit[commom_pre])) {
            (*start)[commom_pre]++;
            start->resize(commom_pre + 1);
            assert(Compare(*start, limit) < 0);
        }
    }
}   


// 没有上端的限制，找到下一个比他大的但是很短的数值用于最后的标识 indexBlock
void ByteWiseComparator::FindShortSuccessor(std::string* key) const  {
        //Find first 
        size_t n = key->size();
        for (int i = 0; i < n; i++) {
            const uint8_t byte = (*key)[i];
            if (byte != static_cast<uint8_t>(0xff)) {
                (*key)[i] = byte + 1;
                key->resize(i + 1);
                return;
            }
        }   
    }

const Comparator* GetByteWiseComparator() {
    static ByteWiseComparator singleton;
    return &singleton;
}


// const Comparator* BytewiseComparator() {
//   static NoDestructor<BytewiseComparatorImpl> singleton;
//   return singleton.get();
// }

}   // namespace xindb

