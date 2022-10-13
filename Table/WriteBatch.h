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

    // 帮助在contents中进行迭代
    Status Iterator(Handler* handler) const;

 private:
    friend class WriteBatchInternal;
    
    std::string rep_;
};

}   // namespace xindb 

#endif