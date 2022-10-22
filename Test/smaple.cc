
#include "DB.h"

#include <assert.h>
#include <iostream>
#include <string.h>
#include <string>

#include "DB_impl.h"
#include "Options.h"
#include "BloomFilter.h"
#include "Status.h"

using namespace xindb;


void sample_write_to_sst() {
    Status status;
    xindb::Options options;
    options.filter_policy = NewBloomFilterPolicy(10);
    options.block_restart_interval = 10;
    std::string dbname = "BING";
    DBImpl* db = new DBImpl(options, dbname);

    std::string key = "age";
    std::string this_year_value = "20";
    std::string next_year_value = "21";
    std::string db_value;

    // 写入 Key, Value
    status = db->Put(WriteOptions(), key, this_year_value);
    assert(status.ok());

    status = db->Put(WriteOptions(), "name", "Jeff Dean");
    status = db->Put(WriteOptions(), "company", "Google");

    // 删除Key|Value, 会在内存中就实现了删除
    status = db->Delete(WriteOptions(), "company");

    db->showMemEntries();
    db->Persistent();
    delete db;
}



void sample_read_from_sst() {
    Options options;
    options.block_restart_interval = 10;
    options.filter_policy = NewBloomFilterPolicy(10);               // 布隆过滤器

    std::string dbname = "BING";
    DBImpl* db = new DBImpl(options, dbname);
    

    std::string tmp_value;
    db->Get(ReadOptions(), "age", &tmp_value);
    std::cout << "Seek value from sstable, should be [20], Real is [" << tmp_value << "]\n";

    db->Get(ReadOptions(), "name", &tmp_value);
    std::cout << "Seek value from sstable, should be [Jeff Dean], Real is [" << tmp_value << "]\n";

    db->Get(ReadOptions(), "company", &tmp_value);
    std::cout << "Seek value from sstable, should be nil, Real is [["    << tmp_value << "]\n";
    return ;
}

int main() {

    sample_write_to_sst();
    std::cout << "=========== Read From SSTable =============\n";
    sample_read_from_sst();
    
    return 0;
}





