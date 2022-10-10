#include <unistd.h>
#include <iostream>
#include "PosixEnv.h"
#include "tableBuilder.h"
#include "Status.h"
#include "PosixWrite.h"


using namespace xindb;

int main() {
    Options options;
    options.block_restart_interval = 4;

    std::string file_name("table_builder.data");
    // PosixWritableFile* file;
    int fd = open(file_name.c_str(), O_RDWR);

    PosixWritableFile file(file_name, fd);
    TableBuilder table_builder(options, &file);

    table_builder.Add("confuse", "value");
    table_builder.Add("contend", "value");
    table_builder.Add("cope", "value");
    table_builder.Add("copy", "value");
    table_builder.Add("corn", "value");

    // 70 bytes
    Status status = table_builder.Finish();


    std::cout << "table_builder, the num entries : " << table_builder.NumEntries() << std::endl;
    std::cout << "table_builder's file size" << table_builder.FileSize()   << std::endl;

    file.Close();

    return 0;

}