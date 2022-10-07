#ifndef XINDB_WAL_LOGWRITER_H
#define XINDB_WAL_LOGWRITER_H

#include "PosixWrite.h"
#include "Logformat.h"
#include "Slice.h"
#include "Status.h"

namespace xindb {

class PosixWritableFile;
namespace Log {

class Writer {
 public:
   explicit Writer(PosixWritableFile* dest);
   
   Writer(PosixWritableFile* dest, uint64_t dest_length);

   Writer(const Writer&) = delete;
   Writer&operator=(const Writer&) = delete;

   ~Writer();

   Status AddRecord(const Slice& slice);
   

 private:
   Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);

   PosixWritableFile* dest_;
   int block_offset_;       // 当前的offset在block中

   uint32_t type_crc_[kMaxRecordType + 1];
};




}   // namespace Log
}   // namespace xindb

#endif