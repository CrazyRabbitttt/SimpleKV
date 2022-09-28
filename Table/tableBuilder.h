#ifndef XINDB_TABLE_TABLEBUILDER_H
#define XINDB_TABLE_TABLEBUILDER_H

#include "Status.h"
#include "Options.h"


namespace xindb {

class BlockBuilder;
class WritableFile;
class BlockHandle;
// class to build table ?
class TableBuilder {

 public:
    TableBuilder(const Options& options, WritableFile* file);

    TableBuilder(const TableBuilder&) = delete;
    TableBuilder& operator=(const TableBuilder&) = delete;

 private:

    // 具体的将block写入到磁盘上面
    void WriteBlock(BlockBuilder* block, BlockHandle* handle);
        
    struct Rep;
    Rep* rep_;
};


} //namespace xindb 


#endif 