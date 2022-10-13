
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include "PosixEnv.h"
#include "Env.h"

#include "tableBuilder.h"
#include "table.h"

using namespace xindb;


void scan_by_table_iterator() {
    Options options;
    options.block_restart_interval = 4;
    Table* table = nullptr;
    std::string filename("table_builder.data");
    int fd = open(filename.c_str(), O_RDWR);
    PosixRandomAccessFile file(filename, fd);

    struct stat file_stat;
    stat(filename.c_str(), &file_stat);

    std::cout << "target file's size : " << file_stat.st_size << std::endl;
    Status status = Table::Open(
        options,
        &file,
        file_stat.st_size,
        &table
    );


    std::cout << "leveldb::Table::Open status: "<< status.ToString() << std::endl;
    Iterator* iter = table->NewIterator(ReadOptions());
    iter->SeekToFirst();

    while (iter->Valid()) {
        std::cout << iter->key().ToString() << "->" << iter->value().ToString() << std:: endl;
        iter->Next();
    }

    delete iter;
    delete table;

}



// int main() {
//     printf("测试读取写入磁盘中的SST... Wish success\n");
//     scan_by_table_iterator();

//     return 0;
// }

