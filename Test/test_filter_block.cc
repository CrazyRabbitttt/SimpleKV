#include <iostream>

#include "BloomFilter.h"
#include "filterBlock.h"
#include "Filter.h"

using namespace xindb;


int main() {
    const FilterPolicy* bloom_filter = NewBloomFilterPolicy(10);
    FilterBlockBuilder filter_block_builder(bloom_filter);


    filter_block_builder.StartBlock(0);


    filter_block_builder.AddKey("Hello");
    filter_block_builder.AddKey("World");
    filter_block_builder.StartBlock(3000);

    filter_block_builder.AddKey("Go");
    filter_block_builder.AddKey("Programmer");
    filter_block_builder.StartBlock(20000);

    filter_block_builder.AddKey("a");
    filter_block_builder.AddKey("b");
    filter_block_builder.AddKey("c");


    Slice result = filter_block_builder.Finish();

    FilterBlockReader filter_block_reader(bloom_filter, result);

    std::cout << filter_block_reader.KeyMayMatch(0, "Hello") << std::endl;  // 1
    std::cout << filter_block_reader.KeyMayMatch(0, "World") << std::endl;  // 1
    std::cout << filter_block_reader.KeyMayMatch(0, "Go")    << std::endl;  // 0

    std::cout << filter_block_reader.KeyMayMatch(3000, "Go") << std::endl;  // 1
    std::cout << filter_block_reader.KeyMayMatch(20000, "b") << std::endl;  // 1 
    std::cout << filter_block_reader.KeyMayMatch(20000, "d") << std::endl;  // 0

    delete bloom_filter;

    return 0;
}

