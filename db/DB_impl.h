#ifndef XINDB_DB_DBIMPL_H
#define XINDB_DB_DBIMPL_H

#include "DB.h"

#include <queue>
#include <atomic>

#include "Mutex.h"
#include "dbFormat.h"
#include "WriteBatch.h"
#include "WriteBatchInternal.h"
#include "Log_writer.h"

namespace xindb {

// 前向声明，减少编译时期的引用
class MemTable;

class DBImpl : public DB {
 public:    

    DBImpl(const Options& options, const std::string& dbname);

    DBImpl(const DBImpl&) = delete;
    DBImpl&operator=(const DBImpl&) = delete;

    ~DBImpl() override;

    Status Get(const ReadOptions& options, const Slice& key, std::string* value) override;

    Status Put(const WriteOptions&, const Slice& key, const Slice& value) override;
    
    Status Delete(const WriteOptions&, const Slice& key) override;

    Status Write(const WriteOptions& options, WriteBatch* updates) override;

    Status BuildTable(const Options& options, WritableFile* file, Iterator* iter);

   //  (const Options& options, WritableFile* file)
    Status showMemEntries();

    Status Persistent();         // 持久化数据到磁盘上面


 private:
    friend class DB;          // DB 作为接口能够直接的访问 DBImpl 的实现内容
    struct Writer;

    Status MakeRoomForWrite(bool force);     // 需要独占锁
    Status WriteLevel0Table(MemTable* mem);  // 写数据到SST【0层中】
    WriteBatch* BuildBatchGroup(Writer** last_writer);

    std::atomic_int sstNumber_;         // assign name of sstable, do not need sync method
    port::Mutex mutex_;                 // 维护并发写入的线程安全
   //  port::CondVar condv_;
    port::CondVar background_work_finished_signal_;   // if background thread finished write
    const Options options_;             // 
    std::string dbname_;                // dbname 
    WriteBatch* tmp_batch_;
   //  log::Writer* log_;
    Log::Writer* log_;
    MemTable* mem_;                     // Memtable 
    MemTable* imm_;                     // ImmMemTable
    std::deque<Writer*> writers_;       // 维护一个 writers 的队列
    const ByteWiseComparator* byte_com_;      // compare 策略
    InternalKeyCom internalcom_;        // Internal Comparator
    PosixWritableFile* file_;           // posix writable file 
    SequencrNumber sequence_;           // Use DBImpl to control the version 

};



}   //namespace xindb 

#endif 