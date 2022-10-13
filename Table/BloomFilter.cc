#include "BloomFilter.h"

namespace xindb {

const FilterPolicy* NewBloomFilterPolicy(int bits_per_key) {
  return new BloomFilterPolicy(bits_per_key);
}


}   // namespace xindb 