#include "PosixWrite.h"
#include "blockBuilder.h"
#include "block.h"
#include "format.h"
#include "Options.h"


#include <iostream>
using namespace xindb;

std::string test_block_builder() {
    Options options;
    options.block_restart_interval = 4;
    BlockBuilder block_builder(&options);

    // sharedlen | nonsharedlen | val len | non-shared key | value |

    // restart = [0]
    //0x00 07 05 c o n f u s e v a l u e
    block_builder.Add("confuse", "value");//12
    //0x03 04 05 t e n d v a l u e
    block_builder.Add("contend", "value");//12
    //0x02 02 05 p e v a l u e
    block_builder.Add("cope", "value");//9
    //0x03 01 05 y v a l u e
    block_builder.Add("copy", "value");//9

    block_builder.Add("corn", "value");
    Slice block_builder_buffer = block_builder.Finish();

    std::string block_contents = block_builder_buffer.ToString();
    std::cout << block_contents << std::endl;
    return block_contents;
}


void test_block(const std::string block_contents) {
    Options options;

    BlockContents block_info;
    block_info.data = block_contents;
    block_info.cacheable = false;
    block_info.heap_allocated = false;          // 栈区分配的数据

    Block block(block_info);
    std::cout << "block size:" << block.size() << std::endl;

    Iterator* block_iter = block.NewIterator(options.comparator);
    block_iter->SeekToFirst();

    while (block_iter->Valid()) {
        std::cout << block_iter->key().ToString()
                  << " -> "
                  << block_iter->value().ToString()
                  << std::endl;
        block_iter->Next();
    }

    block_iter->Seek("corm");
    // seek corm        corn->value
    std::cout << "seek corm\t" << block_iter->key().ToString()
        << " -> "
        << block_iter->value().ToString()
        << std::endl;

    block_iter->SeekToLast();
    //last    corn -> value
    std::cout << "last\t" << block_iter->key().ToString()
        << " -> "
        << block_iter->value().ToString()
        << std::endl;

    delete block_iter;
}





// int main() {


//     // std::string filename("Posix_writable_file_test.data");
//     // int fd = open(filename.c_str(), O_RDWR);
//     // PosixWritableFile file(filename, fd);

//     // file.Append("hello, world\n");
//     // file.Append("hello, go\n");
//     // file.Append("hello, programmer\n");
//     // file.Append(std::string("\x00\x00\x00\x00\x00\x00", 4));

//     // file.Flush();

//     std::string tmp = test_block_builder();
//     test_block(tmp);
//     return 0;
// }