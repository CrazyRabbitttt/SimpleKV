
#include "two_level_iterator.h"
#include "table.h"
#include "block.h"
#include "iterator.h"

namespace xindb {

class TwoLevelIterator : public Iterator{

 public:
    


 private:
    Iterator index_iter_;
    Iterator data_index_;

    Status status_; 

};



}   // namespace xindb