
#include "DB_impl.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <string>

#include "MutexLock.h"
#include "Coding.h"
#include "Status.h"
#include "BloomFilter.h"
#include "tableBuilder.h"
#include "iterator.h"
#include "PosixEnv.h"
#include "table.h"

namespace xindb {

// // const ByteWiseComparator* bytecom = new ByteWiseComparator();
// // InternalKeyCom internalcom(bytecom);
// // MemTable mem(internalcom);


DBImpl::DBImpl(const Options& raw_options, const std::string& dbname)
    : 
      background_work_finished_signal_(&mutex_),
      options_(raw_options),
      dbname_(std::move(dbname)),
      tmp_batch_(new WriteBatch),
      log_(nullptr),
      mem_(new MemTable(internalcom_)), 
      imm_(nullptr),
      byte_com_(new ByteWiseComparator()),
      internalcom_(byte_com_),
      sequence_(1)
      {
        int fd = open(dbname.c_str(), O_RDWR);
        file_ = new PosixWritableFile(dbname, fd);                              // 记得析构的时候 delete 掉
      }

DBImpl::~DBImpl() {
    delete tmp_batch_;
    delete file_;
    delete byte_com_;
    delete mem_;
}



struct DBImpl::Writer {
    explicit Writer(port::Mutex* mu)
        : batch(nullptr), sync(false), done(false), condv(mu)
        {}
 
    Status status;
    WriteBatch* batch;
    bool sync;
    bool done;
    port::CondVar condv;

};



Status DBImpl::MakeRoomForWrite(bool force) {
    // bool allow_delay = !force;
    Status s;
    while (true) {
        // 判断 memtable 中的数据是否是达到了既定的数据
        if (!force && (mem_->ApproximateMemoryUsage() <= options_.write_buffer_size)) {
            // 当前的 MemTable 的大小是小于等于 4MB, 能够进行写入操作
            break;
        } else if (imm_ != nullptr) {
            // 需要等待 Immutable MemTable 进行刷盘
        } else {    
            // 交换 memtable & immutable memtable【空的】 

        }
    }
    return s; 
}


 // 将数据进行持久化 dump 到磁盘上面去
Status DBImpl::Persistent() {
    TableBuilder* table_ = new TableBuilder(options_, file_);
    Iterator* iter = mem_->NewIterator();           // get the iter of memtable 
    iter->SeekToFirst();                            // run to the first position of memtable 
    // Slice pre_key = "dytez.!@#$%#&*#-="; 
    for (; iter->Valid(); iter->Next()) {
        // if (pre_key.size() == iter->key().size() && memcmp(pre_key.data(), iter->key().data(), pre_key.size()) == 0) {
        //     printf("重复的key插入，跳过\n");
        //     continue;           // 不能将重复的数据刷新到磁盘上面
        // } else {
        //     printf("Key 是不重复的，能够直接插入\n");
        // }
        // printf("The key to add : [%s]\n", iter->key());
        table_->Add(iter->key(), iter->value());
        // pre_key = iter->key();
    }

    Status status = table_->Finish();
    delete iter;
    delete table_;
    return status;
}


Status DBImpl::Write(const WriteOptions& option, WriteBatch* batch) {
    Status status;
    // (1) init the write 
    // Writer w(&mutex_);
    // w.batch = batch;
    // w.sync  = option.sync;
    // w.done  = false; 

    // // (2) queuing the task 临界区
    // MutexLock lock(&mutex_);
    // writers_.push_back(&w);                         // 将writer放进入到 queue 中
    // while (!w.done && &w != writers_.front()) {     // 没有写完并且还没有轮到你
    //     w.condv.Wait();
    // }
    // if (w.done) {
    //     return w.status;            // 写入的状态
    // }

    // (3) 
    // 写日志
    // Status status = log_->AddRecord(WriteBatchInternal::Contents(batch));

    // 写入到 MemTable 中
    status =  WriteBatchInternal::InsertInto(batch, mem_);

    return status;
}



// 遍历输出一下 memtable 中的数据，看看是否是正确的
Status DBImpl::showMemEntries() {
    // 1.获得 MemTable 的迭代器
    Iterator* iter = mem_->NewIterator();

    iter->SeekToFirst();
    if (iter->Valid()) {
        for (; iter->Valid(); iter->Next()) {
            printf("From MemTable: [%s]->[%s]\n", iter->key().data(), iter->value().data());
        }
    }
    delete iter;
    return Status::OK();
}

Status DBImpl::Get(const ReadOptions& options, const Slice& key, std::string* value) {
    // 从 Table 中 Get value出来
    // options_.block_restart_interval = 4;
    // options_.filter_policy = NewBloomFilterPolicy(10);
    Table* table = nullptr;
    std::string filename = "BING";

    int fd = open(filename.c_str(), O_RDWR);
    PosixRandomAccessFile file(filename, fd);

    struct stat file_stat;
    stat(filename.c_str(), &file_stat);


    Status status = Table::Open(
        options_,
        &file,
        file_stat.st_size,
        &table
    );

    Iterator* iter = table->NewIterator(options);
    // iter->SeekToFirst();                    // find the first entry of SSTable
    // while (iter->Valid()) {
    //     std::cout << "From SSTable : " << iter->key().ToString() << "->" << iter->value().ToString() << std:: endl;        
    //     iter->Next();
    // }

    iter->Seek(key);        // now the 
    std::string tmp(iter->value().data());
    *value = std::move(tmp);
    delete iter;
    delete table;
    return Status::OK();
}


Status DBImpl::Delete(const WriteOptions& option, const Slice& key) {
    WriteBatch batch;
    WriteBatchInternal::SetSequence(&batch, sequence_++);
    batch.Delete(key);
    return Write(option, &batch);
}
Status DBImpl::Put(const WriteOptions& option, const Slice& key, const Slice& value) {
    WriteBatch batch;
    WriteBatchInternal::SetSequence(&batch, sequence_++);             // 用DBIpml自己进行 sequence 的控制
    batch.Put(key, value);
    return Write(option, &batch);
}


Status DB::Put(const WriteOptions& option, const Slice& key, const Slice& value) {
    WriteBatch batch;
    batch.Put(key, value);
    return Write(option, &batch);
}


Status DB::Delete(const WriteOptions& opt, const Slice& key) {
    WriteBatch batch;
    batch.Delete(key);
    return Write(opt, &batch);
}


// 创建 batch group
WriteBatch* DBImpl::BuildBatchGroup(Writer** last_writer) {
    Writer* first = writers_.front();
    WriteBatch* result = first->batch;
    assert(result != nullptr);

    size_t size = WriteBatchInternal::ByteSize(first->batch);

    // 避免小的块写的太多？

    size_t max_size = 1 << 20;
    if (size <= (128 << 10)) {
        max_size = size + (128 << 10);
    }

    *last_writer = first;
    std::deque<Writer*>::iterator iter = writers_.begin();
    ++iter;
    for (; iter != writers_.end(); ++iter) {        // 遍历 writers
        Writer* w = *iter;
        if (w->sync && !first->sync) {
            break;
        }

        if (w->batch != nullptr) {
            size += WriteBatchInternal::ByteSize(w->batch);
            if (size > max_size) {
                break;
            }

            // Append to *result 
            if (result == first->batch) {
                result = tmp_batch_;
                assert(WriteBatchInternal::Count(result) == 0);
                WriteBatchInternal::Append(result, w->batch);
            }
            WriteBatchInternal::Append(result, w->batch);
        }
        *last_writer = w;
    }
    return result;
}

// void DBImpl::void SetOptionsfilter(const FilterPolicy*);


}   // namespace xindb 







