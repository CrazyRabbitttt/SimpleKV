#include "Options.h"
#include "Comparator.h"

using namespace xindb;


// Options::Options()
//     : comparator(GetByteComparator()) {}


Options::Options() 
    : comparator(new ByteWiseComparator()) {}



