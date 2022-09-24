
#include "PosixWrite.h"
#include "Log_writer.h"

#include <fcntl.h>

using namespace xindb;
using namespace xindb::Log;

int main(int argc, char** argv) {

    char* filename = "testLog";
    int fd = open(filename, O_RDWR);            // 按照能够进行读写的方式打开文件

    PosixWritableFile dest(std::string(filename), fd);
    Writer writer(&dest);                       


    // now can add record to the Logfile

    for (int i = 0; i < 20; i++) {
        Slice tmpslice(std::string('a'+ i, 4));
        writer.AddRecord(tmpslice);             // 将数据写到文件中去
    }

    printf("The end of the testing of Logfile\n");
    return 0;

}

