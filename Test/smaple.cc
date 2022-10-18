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

    // std::string key2 = "DayDreamer";
    // std::string real1 = "ShaoGuixin";
    Status status = db->Put(WriteOptions(), key, val_age);
    status = db->Put(WriteOptions(), name, real);
    status = db->Put(WriteOptions(), key1, value1);

    printf("=======================\n");
    db->showMemEntries();
    db->Persistent();

    
    delete db;
}


