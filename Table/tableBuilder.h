#ifndef XINDB_TABLE_TABLEBUILDER_H
#define XINDB_TABLE_TABLEBUILDER_H

#include "Status.h"
#include "Options.h"


namespace xindb {

class WritableFile;

// class to build table ?
class TableBuilder {

 public:
    TableBuilder(const Options& options, WritableFile* file);

    TableBuilder(const TableBuilder&) = delete;
    TableBuilder& operator=(const TableBuilder&) = delete;

 private:

    

    struct Rep;
    Rep* rep_;
};


} //namespace xindb 


#endif 