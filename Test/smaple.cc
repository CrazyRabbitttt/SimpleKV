#include <assert.h>
#include "DB.h"
#include "DB_impl.h"
#include "Options.h"
#include "Status.h"

using namespace xindb;

int main() {
    // xindb::DB* db = nullptr;
    
    xindb::Options options;
    std::string dbname = "XXXXXIN";
    DBImpl* db = new DBImpl(options, dbname);

    std::string key  = "Zage";
    std::string val_age = "20008";

    std::string name = "DayDreamer";
    std::string real = "shaoguixin";

    std::string key1   = "key3";
    std::string value1 = "value3";
    Status status = db->Put(WriteOptions(), key, val_age);
    assert(status.ok());

    status = db->Put(WriteOptions(), name, real);
    assert(status.ok());

    status = db->Put(WriteOptions(), key1, value1);
    // printf("put kv's into memtable...\n");

    printf("=======================\n");
    db->showMemEntries();

    
    delete db;
}


