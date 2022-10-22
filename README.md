***一个简单的 Key-value 的存储引擎***

- [x] 使用 `SkipList` 在内存中组织数据
- [x] `WAL` 保证数据不会丢失
- [x] `SSTable` 架构组织数据， 持久化到磁盘中

***运行***

> 下载源码到本地之后运行`Run.sh` 即可一键编译

### 测试文件

> - ***sample***: 简单测试整体的功能
> - ***test_filter_block***: 测试 ***布隆过滤器*** 是否成功 【内存中】
> - ***testTableBuilder***: 测试 **写入Table**到磁盘中，上面是 `hexdump` 数据格式 【写入磁盘】
> - ***testTableReader:*** 测试 **读取上面写入的table**，然后迭代器遍历数据 【从磁盘读】
> - ***testblock***: 测试 **block的生成、迭代遍历等** 【内存中】
> - ***testSkipList***: 测试 跳表是否成功 【内存中】

### table_builder 插入数据后磁盘内容

```sh
Add: {"confuse", "value"} {"contend", "value"}, {"cope", "value"}, {"copy", "value"}, {"corn", "value"}	
============================================================================================================
00000000  00 07 05 63 6f 6e 66 75  73 65 76 61 6c 75 65 03  |...confusevalue.|
00000010  04 05 74 65 6e 64 76 61  6c 75 65 02 02 05 70 65  |..tendvalue...pe|
00000020  76 61 6c 75 65 03 01 05  79 76 61 6c 75 65 00 04  |value...yvalue..|
00000030  05 63 6f 72 6e 76 61 6c  75 65 00 00 00 00 2e 00  |.cornvalue......|
00000040  00 00 02 00 00 00 01 e8  e5 ed 99 00 87 93 42 0b  |..............B.|
00000050  07 43 23 06 00 00 00 00  09 00 00 00 0b 00 47 af  |.C#...........G.|
00000060  7e f1 00 12 02 66 69 6c  74 65 72 2e 42 6c 6f 6f  |~....filter.Bloo|
00000070  6d 46 69 6c 74 65 72 4b  12 00 00 00 00 01 00 00  |mFilterK........|
00000080  00 01 4e 6a 85 24 00 01  02 64 00 46 00 00 00 00  |..Nj.$...d.F....|
00000090  01 00 00 00 01 8f c6 2f  15 62 1f 86 01 0e 00 00  |......./.b......|
000000a0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
000000c0  00 57 fb 80 8b 24 75 47  db                       |.W...$uG.|
000000c9
```



### 简单的测试样例程序

```cpp
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
```

### 运行结果

```sh
From MemTable: [age]->[20]
From MemTable: [company]->[]
From MemTable: [company]->[Google]
From MemTable: [name]->[Jeff Dean]
Write MetaBlock, the filter policy name : [BloomFilter]
=========== Read From SSTable =============
Seek value from sstable, should be [20], Real is [20]
Seek value from sstable, should be [Jeff Dean], Real is [Jeff Dean]
Seek value from sstable, should be nil, Real is []
```



## 优化设计：

### MemTableIterator 的设计

> - ***MemTable*** 内部使用 **skiplist** 进行维护数据
> - ***MemtableIterator*** 作为 **memtable** 的友元类出现，为了访问 **skiplist** 

怎么创建 ***memtableiterator*** 呢？ **MemTable** 中有一个成员函数 `Iterator* NewIterator()` 用来创建本 **memtable** 的迭代器，传入 ***table_*** 生成迭代器类



### 🐛:

1. 不要在 头文件中 定义函数，可以声明但是不要在头文件中定义非类的成员函数，在.cc文件中定义

2. IteratorWrapper 构造函数传入iter的时候忘记调用 Set(), 导致上层的 iter 一直是空的
