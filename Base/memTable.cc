#include "memTable.h"

#include "Coding.h"
#include "InterCom.h"

using namespace xindb;

static Slice GetLengthPrefixedSlice1(const char* data) {
  uint32_t len;
  const char* p = data;
  p = GetVarint32Ptr(p, p + 5, &len);  // +5: we assume "p" is not corrupted
  return Slice(p, len);
}


MemTable::MemTable(const InternalKeyCom& comparator)
    : refs_(0), comparator_(comparator),  table_(comparator_, &arena_) {}

MemTable::~MemTable() {
    assert(refs_ == 0);
}

size_t MemTable::ApproximateMemoryUsage() {
    return arena_.Memoryusage(); 
}


int MemTable::KeyComparator::operator()(const char* ptra, const char* ptrb) const {
    // 传入的是InternalKey?
    Slice a = GetLengthPrefixedSlice1(ptra);
    Slice b = GetLengthPrefixedSlice1(ptrb);
    return comparator.Compare(a, b);
}


void MemTable::Add(SequencrNumber seq, ValueType type, const Slice& key, const Slice& value) {
    // 将这些的参数传进来，然后插入到SkipList中去
    // key_size, key_bytes, tag, value_size, value_bytes

    size_t key_size   = key.size();
    size_t value_size = value.size();
    size_t internal_key_size = key_size + 8;            // 8是固定格式tag的大小

    const size_t encoded_len = VarintLength(internal_key_size) + 
                                internal_key_size + VarintLength(value_size) +
                                value_size;
    char* buf = arena_.Allocate(encoded_len);
    // 下面将数据压缩到buf中
    char* p = EncodeVarint32(buf, internal_key_size);
    memcpy(p, key.data(), key_size);
    p += key_size;
    EncodeFixed64(p, (seq << 8) | type);
    p += 8;
    p = EncodeVarint32(p, value_size);
    memcpy(p, value.data(), value_size);

    assert(p + value_size == buf + encoded_len);
    // 插入到SkipList中
    table_.Insert(buf);
}


// 从跳表当中拿出来
bool MemTable::Get(const LookUpKey& key, std::string* value, Status* status) {
    Slice memkey = key.memtable_key();      // internalkeysize + internalkey
    Table::Iterator iter(&table_);          // SkipList的迭代器
    iter.Seek(memkey.data());
    if (iter.Valid()) {
        // klen, userkey, tag, vlen, value
        const char* entry = iter.key();
        uint32_t key_len;
        // 进行32位可变数据的Decode，获得指针位置
        const char* key_ptr = GetVarint32Ptr(entry, entry + 5, &key_len);
        if (comparator_.comparator.user_comparator()->Compare(          // 如果说key的值是相同的
            Slice(key_ptr, key_len - 8), key.user_key()) == 0) {
            // 正确的User Key
            const uint64_t tag = DecodeFixed64(key_ptr + key_len - 8);
            switch (static_cast<ValueType>(tag & 0xff)) {
                case kTypeValue : {
                    Slice v = GetLengthPrefixedSlice1(key_ptr + key_len);        // 获得value
                    value->assign(v.data(), v.size());
                    return true;
                }
                case kTypeDeletion :        // value是已经被删除了的
                    *status = Status::NotFound(Slice());
                    return true;
            }
        }
    }
    return false;
}



















