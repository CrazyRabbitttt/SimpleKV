#include <iostream>

#include "Log_writer.h"
#include "PosixWrite.h"
#include "CrcChecksum.h"
#include "Coding.h"

using namespace xindb;
using namespace xindb::Log;

// int main() {
//     std::string filename("log_write.data");
//     int fd = open(filename.c_str(), O_RDWR);
//     PosixWritableFile file(filename, fd);

//     Writer write(&file);
//     const std::string data = "HelloWorld";

//     uint32_t crcnum = tinycrc::crc32(data.data(), data.size());
//     std::cout << " crc check sum : " << crcnum << std::endl;
//     write.AddRecord(data);
//     return 0;
// }