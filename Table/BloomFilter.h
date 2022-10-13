#ifndef XINDB_TABLE_BLOOMFILTER_H
#define XINDB_TABLE_BLOOMFILTER_H

#include "Slice.h"
#include "Filter.h"         // 无语了，我以为文件名是filterPolicy 
#include "Hash.h"

namespace xindb {

static uint32_t BloomHash(const Slice& key) {
    return Hash(key.data(), key.size(), 0xbc9f1d34);
}


class BloomFilterPolicy : public FilterPolicy{
 public:
    explicit BloomFilterPolicy(int bits_per_key) 
        : bits_per_key_(bits_per_key)
    {
       k_ = static_cast<size_t>(bits_per_key * 0.69);  // ln2 ? 
       if (k_ < 1)  k_ = 1;
       if (k_ > 30) k_ = 30;
    }

    const char* Name() const override { return "BloomFilter"; }

    void CreateFilter(const Slice* keys, int n, std::string* dst) const override {
        // 计算 bloom 所需要的bit And bytes
        size_t bits = n * bits_per_key_;

        // 如果说位数太少了，假阳性的可能性是很高的
        if (bits < 64) bits = 64;

        size_t bytes = (bits + 7) / 8;      // 向上取整
        bits = bytes * 8;

        // 下面就是双哈希然后更新 bitmap
        const size_t init_size = dst->size();
        dst->resize(init_size + bytes, 0);
        dst->push_back(static_cast<char>(k_));  // Remember # of probes in filter
        char* array = &(*dst)[init_size];
        for (int i = 0; i < n; i++) {
        // Use double-hashing to generate a sequence of hash values.
        // See analysis in [Kirsch,Mitzenmacher 2006].
        uint32_t h = BloomHash(keys[i]);
        const uint32_t delta = (h >> 17) | (h << 15);  // Rotate right 17 bits
        for (size_t j = 0; j < k_; j++) {
            const uint32_t bitpos = h % bits;
            array[bitpos / 8] |= (1 << (bitpos % 8));
            h += delta;
        }
        }
    }

    bool KeyMayMatch(const Slice& key, const Slice& bloom_filter) const override {
        const size_t len = bloom_filter.size();
        if (len < 2) return false;

        const char* array = bloom_filter.data();
        const size_t bits = (len - 1) * 8;

        // Use the encoded k so that we can read filters generated by
        // bloom filters created using different parameters.
        const size_t k = array[len - 1];
        if (k > 30) {
        // Reserved for potentially new encodings for short bloom filters.
        // Consider it a match.
        return true;
        }

        uint32_t h = BloomHash(key);
        const uint32_t delta = (h >> 17) | (h << 15);  // Rotate right 17 bits
        for (size_t j = 0; j < k; j++) {
        const uint32_t bitpos = h % bits;
        if ((array[bitpos / 8] & (1 << (bitpos % 8))) == 0) return false;
        h += delta;
        }
        return true;

    }

 private:
    size_t bits_per_key_;                   // 每个Key的位数, 【M/N】
    size_t k_;                              // the number of hash func
};

const FilterPolicy* NewBloomFilterPolicy(int bits_per_key);


}   // namesapce xindb 

#endif