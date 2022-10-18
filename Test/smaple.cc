
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
        // xindb::DB* db = nullptr;
    
    xindb::Options options;
    options.filter_policy = NewBloomFilterPolicy(10);
    options.block_restart_interval = 4;
    std::string dbname = "BING";
    DBImpl* db = new DBImpl(options, dbname);

    std::string key  = "Zage";
    std::string val_age = "20008";

    std::string name = "DayDreamer";
    std::string real = "shaoguixin";

    std::string key1   = "key3";
    std::string value1 = "value3";

    Status status = db->Put(WriteOptions(), key, val_age);
    status = db->Put(WriteOptions(), name, real);
    status = db->Put(WriteOptions(), key1, value1);

    std::string tmpkey   = "key";
    std::string tmpvalue = "value";
    for (int i = 10; i < 23; i++) {
        status = db->Put(WriteOptions(), Slice(tmpkey + std::to_string(i)), Slice(tmpvalue + std::to_string(i)));
    }


    printf("=======================\n");
    db->showMemEntries();
    db->Persistent();
    delete db;
}


void sample_read_from_sst() {
    Options options;
    options.block_restart_interval = 4;
    options.filter_policy = NewBloomFilterPolicy(10);               // 布隆过滤器

    std::string dbname = "table_builder.data";
    DBImpl* db = new DBImpl(options, dbname);
    
    std::string key  = "Zage";
    std::string val_age;
    // db->Delete(WriteOptions(), "key12");
    db->Get(ReadOptions(), key, &val_age);
    std::cout << "Seek value from sstable, should be 20008, Real is [" << val_age << "]\n";

    
    for (int i = 10; i < 23; i++) {
        std::string target_value;
        std::string search_key = std::string("key") + std::to_string(i);
        db->Get(ReadOptions(), Slice(search_key), &target_value);

        std::cout << "Seek value from sstable ";
        printf("[%s] -> [%s]\n", search_key.data(), target_value.data());
    }
    return ;
}

int main() {

    // sample_write_to_sst();
    sample_read_from_sst();
    
    return 0;
}





