#ifndef XINDB_TABLE_WRITEBATCH_H_
#define XINDB_TABLE_WRITEBATCH_H_

#include <string>

namespace xindb {

class Slice;

class WriteBatch {
 public:
    class Handler {
     public:
        virtual ~Handler();
        virtual void Put(const Slice& key, const Slice& value) = 0;
        virtual void Delete(const Slice& key) = 0;
    }; 

    WriteBatch();

    WriteBatch(const WriteBatch&) = default;
    WriteBatch& operator=(const WriteBatch&) = default;

    ~WriteBatch();

    void Put(const Slice& key, const Slice& value);

    void Delete(const Slice& key);

    void Clear();
   
    void Append(const WriteBatch& source);

   // 大约的数据的 Size 
    size_t ApproximateSize() const { return rep_.size(); }

    // 传入初始化好了的 memtableinserter 将 rep_ 中存储的 entry 写到 memtable 中
    Status Iterate(Handler* handler) const;

 private:
    // 帮助操作内部的数据，如 count & sequence number 
    friend class WriteBatchInternal;
    
    std::string rep_;
};

// helper class, help insert values to memtable 
class MemTableInsert : public WriteBatch::Handler {
 public:
   SequencrNumber sequence_;
   MemTable* mem_;

   // 前面已经是将 memtable 同 writebatch 相联系起来， 调用对应的 memtable 插入数据的接口
   void Put(const Slice& key, const Slice& value) override {
      mem_->Add(sequence_, kTypeValue, key, value);
      sequence_++;
   }

   void Delete(const Slice& key) override {
      mem_->Add(sequence_, kTypeDeletion, key, Slice());    // null value 
      sequence_++;
   }
};


}   // namespace xindb 

#endif