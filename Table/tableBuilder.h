#ifndef XINDB_TABLE_TABLEBUILDER_H
#define XINDB_TABLE_TABLEBUILDER_H

#include "Status.h"
#include "Options.h"


namespace xindb {

class BlockBuilder;
class WritableFile;
class BlockHandle;

class TableBuilder {

 public:
    TableBuilder(const Options& options, WritableFile* file);
   
    // no copy
    TableBuilder(const TableBuilder&) = delete;
    TableBuilder& operator=(const TableBuilder&) = delete;

    ~TableBuilder();

    // 操作的状态
    Status status() const;

    void Add(const Slice& key, const Slice& value);

    void Flush();

    bool ok() const { return status().ok(); }

    Status Finish();

    uint64_t NumEntries() const;

 private:

    void WritaRawBlock(const Slice& block_contents, CompressType type, BlockHandle* handle);

    // 具体的将block写入到磁盘上面, 序列化需要写入的 DataBlock
    void WriteBlock(BlockBuilder* block, BlockHandle* handle);
        
    struct Rep;
    Rep* rep_;
};


} //namespace xindb 


#endif 