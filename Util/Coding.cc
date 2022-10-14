#include "Coding.h"

namespace xindb {

    //进行可变参数Int32的压缩 -> dst 
    char* EncodeVarint32(char* dst, uint32_t v) {
        //reinterpret_cast:可以将指针看作是整数
        uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
        static const int B = 128;      //1 000 | 0000
        if (v < (1 << 7)) {
            *(ptr++) = v;               //最高位就是0， 1Byte
        } else if (v < (1 << 14)) {
            *(ptr++) = v | B;           //没有结束，最高位补1
            *(ptr++) = v >> 7;          //将剩下的填充上去
        } else if (v < (1 << 21)) {
             *(ptr++) = v | B;
            *(ptr++) = (v >> 7) | B;
            *(ptr++) = v >> 14;
        } else if (v < (1 << 28)) {
            *(ptr++) = v | B;
            *(ptr++) = (v >> 7) | B;
            *(ptr++) = (v >> 14) | B;
            *(ptr++) = v >> 21;
        } else {
            *(ptr++) = v | B;
            *(ptr++) = (v >> 7) | B;
            *(ptr++) = (v >> 14) | B;
            *(ptr++) = (v >> 21) | B;
            *(ptr++) = v >> 28;
        }
        return reinterpret_cast<char*>(ptr);

    }

    void PutVarint32(std::string* dst, uint32_t v) {
        char buf[5];
        char* ptr = EncodeVarint32(buf, v);
        dst->append(buf, ptr - buf);    //将压缩过的内容写到dst中去
    }

    void PutFixed32(std::string* dst, uint32_t value) {
        char buf[sizeof(value)];
        EncodeFixed32(buf, value);
        dst->append(buf, sizeof(buf));
    }


    void PutFixed64(std::string* dst, uint64_t v) {
        char buf[sizeof(v)];
        EncodeFixed64(buf, v);
        dst->append(buf, sizeof(buf));
    }


    char* EncodeVarint64(char* dst, uint64_t v) {
        static const int B = 128;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
        while(v >= 128) {
            *(ptr++) = v | B;
            v >>= 7;            //因为一个字节中，最终有效的是后面7位，首位是Valid位
        }
        *(ptr++) = static_cast<uint8_t>(v);
        return reinterpret_cast<char*>(ptr);
    }

    void PutVarint64(std::string* dst, uint64_t value) {
        char buf[10];
        char* ptr = EncodeVarint64(buf, value);
        dst->append(buf, ptr - buf);
    }


    //返回需要压缩数据的len，7位为一组，因为1字节存储7位
    int VarintLength(uint64_t value) {
        //1 2 4 8 16 32 64 128
        int len = 1;
        while (value >= 128) {
            value >>= 7;
            len++;
        }
        return len;
    }

    // 只是针对 uint32_t, 首先写 size ， 然后写 value
    void PutLengthPrefixedSlice(std::string* dst, const Slice& value) {
        PutVarint32(dst, value.size());
        dst->append(value.data(), value.size());
    }

    // 从带有前缀 size 封装好的数据中读取出来 value 
    bool GetLengthPrefixedSlice(Slice* input, Slice* result) {
        uint32_t len;
        if (GetVarint32(input, &len) && input->size() >= len) {
            *result = Slice(input->data(), len);        // 获得了len长度的数据了
            input->remove_prefix(len);
            return true;
        } else {
            return false;                   // value 的size比较小？内部出现了错误
        }
    }

    //进行32-bit整数的解码操作，写到value中去,返回的是解码之后的指针的位置
    const char* GetVarint32PtrFallBack(const char* p, const char* limit, uint32_t* value) {
        uint32_t result = 0;
        //因为写入的时候有效位是7位/字节，所以步长是7
        for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
            uint32_t byte = *(reinterpret_cast<const uint8_t*>(p));     //获取到一个字节的数据，按照uint8的方式解读
            p++;
            if (byte & (1 << 7)) {
                //the largest bit is 1, means more 
                result |= ((byte & 127) << shift);      //去掉最高位置的1
            } else {        
                //No more, the last 
                result |= (byte << shift);
                *value = result;
                return reinterpret_cast<const char*>(p);
            }
        }
        return nullptr;
    }

    bool GetVarint32(Slice* input, uint32_t* value) {
        const char* p = input->data();
        const char* limit = p + input->size();
        const char* q = GetVarint32Ptr(p, limit, value);
        if (q == nullptr) {
            return false;
        } else {
            //将input的指针移除一下internal_size
            *input = Slice(q, limit - q);
            return true;
        }
    }


//基本同上：varint32 
    const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* value) {
  uint64_t result = 0;
  for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7) {
    uint64_t byte = *(reinterpret_cast<const uint8_t*>(p));
    p++;
    if (byte & 128) {
      // More bytes are present
      result |= ((byte & 127) << shift);
    } else {
      result |= (byte << shift);
      *value = result;
      return reinterpret_cast<const char*>(p);
    }
  }
  return nullptr;
}

bool GetVarint64(Slice* input, uint64_t* value) {
  const char* p = input->data();
  const char* limit = p + input->size();
  const char* q = GetVarint64Ptr(p, limit, value);
  if (q == nullptr) {
    return false;
  } else {
    *input = Slice(q, limit - q);
    return true;
  }
}




}   // namespace xindb 