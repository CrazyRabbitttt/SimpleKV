#ifndef XINDB_MEM_MEMTABLE_H
#define XINDB_MEM_MEMTABLE_H

#include "skipList.h"
#include "Status.h"
#include "InterCom.h"
#include "dbFormat.h"

namespace xindb {



class MemTable {
 public:
    
    explicit MemTable(const InternalKeyCom& comparator);

    ~MemTable();

    MemTable(const MemTable&) = delete;
    MemTable& operator=(const MemTable&) = delete;

    void Add(SequencrNumber seq, ValueType type, const Slice& key, const Slice& value);

    bool Get(const LookUpKey& key, std::string* value, Status* status);

    void Ref() { ++refs_; }

    void Unref() { 
        --refs_;
        assert(refs_ >= 0);
        if (refs_ <= 0) delete this;
    }

    size_t ApproximateMemoryUsage();

 private:
    struct KeyComparator {
        const InternalKeyCom comparator;
        explicit KeyComparator(const InternalKeyCom& c) : comparator(c) {}
        int operator() (const char* a, const char* b) const;
    };

    using Table = SkipList<const char*, KeyComparator>;

    int refs_;
    KeyComparator comparator_;
    Table table_;       // 跳表
    Arena arena_;
};
}   // namespace xindb

#endif