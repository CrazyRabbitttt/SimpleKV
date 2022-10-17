
#include "DB_impl.h"
#include "MutexLock.h"
#include "Status.h"
#include "tableBuilder.h"
#include "iterator.h"

namespace xindb {

DBImpl::DBImpl(const Options& raw_options, const std::string& dbname)
    : mem_(nullptr), 
      imm_(nullptr),
      dbname_(dbname),
      background_work_finished_signal_(&mutex_),
      log_(nullptr),
      tmp_batch_(new WriteBatch)
      {
        int fd = open(dbname.c_str(), O_RDWR);
        file_ = new PosixWritableFile(dbname, fd);                              // 记得析构的时候 delete 掉
      }

DBImpl::~DBImpl() {
    delete tmp_batch_;
    delete file_;
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
    bool allow_delay = !force;
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


Status DBImpl::Write(const WriteOptions& option, WriteBatch* batch) {
    Status s;
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
    Status status = log_->AddRecord(WriteBatchInternal::Contents(batch));

    // 写入到 MemTable 中
    status =  WriteBatchInternal::InsertInto(batch, mem_);

    // 将数据持久化到磁盘上面
    // 1. 获得 MemTable's iterator 
    Iterator* iter = mem_->NewIterator();

    // 2. 创建 tablebuilder

    TableBuilder* builder = new TableBuilder(options_, file_);

    iter->SeekToFirst();        // 首先找到 MemTable 的第一个位置
    
    if (iter->Valid()) {        // 如果说是有数据的，直接遍历所有的数据好了
        for (; iter->Valid(); iter->Next()) {
            Slice key   = iter->key();
            Slice value = iter->value();
            printf("From Memtable: key[%s], value[%s]\n", key.data(), value.data()); 
            builder->Add(key, value);
        }
    } 
    s = builder->Finish();              // Finish build the sstable 

    return s;
}


Status DBImpl::Put(const WriteOptions& option, const Slice& key, const Slice& value) {
    return DB::Put(option, key, value);
}


Status DB::Put(const WriteOptions& option, const Slice& key, const Slice& value) {
    WriteBatch batch;
    batch.Put(key, value);
    return Write(option, &batch);
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


}   // namespace xindb 







