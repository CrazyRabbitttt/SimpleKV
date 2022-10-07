#ifndef XINDB_TABLE_TABLE_H
#define XINDB_TABLE_TABLE_H

namespace xindb {

class Table {


 private:   
    struct Rep;

    Rep* const rep_;            // 内部的实现

};

}   // namespace xindb

#endif