#ifndef XINDB_UTIL_HASH_H_
#define XINDB_UTIL_HASH_H_

#include <cstddef>
#include <cstdint>

namespace xindb {

uint32_t Hash(const char* data, size_t n, uint32_t seed);

}   // namesapce xindb

#endif