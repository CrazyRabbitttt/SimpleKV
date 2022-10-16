#ifndef XINDB_DB_DB_H_
#define XINDB_DB_DB_H_


#include "Status.h"
#include "iterator.h"
#include "WriteBatch.h"
#include "Options.h"

namespace xindb {

class DB {
 public:
    /*
        Open database with the specified "name".
        贮存一个指针到堆区开辟的空间 *dbptr 中

    */

   static Status Open(const Options& options, const std::string& name, DB** dbptr);

    DB() = default;

    DB(const DB&) = delete;
    DB&operator=(const DB&) = delete;

    virtual ~DB() = default;

    virtual Status Put(const WriteOptions& options, const Slice& key, const Slice& value) = 0;

    virtual Status Delete(const WriteOptions& options, const Slice& key) = 0;

    // 将指定的 batchs 写进 databases
    virtual Status Write(const WriteOptions& options, WriteBatch* batchs) = 0;

    virtual Status Get(const ReadOptions& options, const Slice& key, std::string* value) =  0;


};  




}   // namespace xindb 

#endif