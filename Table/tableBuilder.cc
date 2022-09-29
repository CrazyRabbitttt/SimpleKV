#include "tableBuilder.h"
#include "Options.h"
#include "blockBuilder.h"

using namespace xindb;


// tableBuilder中保存着所有的变量
struct TableBuilder::Rep {


    Options options_;
    Options index_block_options_;
    WritableFile* file_;                    // Posix写文件
    uint64_t offset_;                       // datablock在SST文件中的偏移量
    Status status_;                         // 操作的状态

    BlockBuilder data_block_;               // dataBlock
    BlockBuilder index_block_;              // IndexBlock

    std::string last_key_;                  // 当前datablock中最后一个写的key
    int64_t num_entries_;                   // 当前datablock的写入数量
    bool closed_;                           // 是否是Finish() Or Abandon()被调用了

};


TableBuilder::TableBuilder(const Options& options, WritableFile* file) 
    :


