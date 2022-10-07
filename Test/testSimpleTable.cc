

#include "tableBuilder.h"
#include "Options.h"
#include "Comparator.h"
#include "PosixWrite.h"
#include "blockBuilder.h"
#include <iostream>

using namespace xindb;




void tablefunc() {

    // int fd = fopen("Table_testfd", "r+");
    int fd = open("Table_testfd", O_RDWR);

    Options options;                    // have default arguments, 全局的变量，最终会将 bytecomparator 进行析构的
    PosixWritableFile file_("Table_testfd", fd);
    TableBuilder tablebuild(options, &file_);

    for (int i = 0; i < 10; i++) {
        std::string key = std::to_string(i);            // 成为 Key
        std::string tmpvalue = key + "val" + std::to_string(i);
        Slice S_key(key), S_value(tmpvalue);
        tablebuild.Add(S_key, S_value);                      // 将数据写进 SST 中
    }
    tablebuild.Flush();
    tablebuild.Finish();     
}






// int main(int argc, char** argv) {
//     // tablefunc();
//     Options options;
//     options.block_restart_interval = 4;

//     std::string filename("table_builder.data");
//     int fd = open(filename.c_str(), O_RDWR);
//     PosixWritableFile file(filename, fd);
    
//     TableBuilder table_builder(options, &file);
//     table_builder.Add("confuse", "value");
//     table_builder.Add("contend", "value");
//     table_builder.Add("cope", "value");
//     table_builder.Add("copy", "value");
//     table_builder.Add("corn", "value");


//     Status status = table_builder.Finish();

//     std::cout << " numentrys: " << table_builder.NumEntries() << std::endl;
//     file.Close();

//     return 0;
// }


