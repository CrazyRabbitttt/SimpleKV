
#include "filterBlock.h"
#include "Filter.h"

#include "Coding.h"

using namespace xindb;

// DataBlock 中每 2KB 就生成一个布隆过滤器
static const size_t kFilterBaselg = 11;
static const size_t kFilterBase    = 1 << kFilterBaselg;  


FilterBlockBuilder::FilterBlockBuilder(const FilterPolicy* policy)
    :policy_(policy) {}


// 使用 ADD 将数据写入之后开始创建 filterblock, 需要传入 datablock 的偏移量来计算 filter 的数目
void FilterBlockBuilder::StartBlock(uint64_t block_offset) {
    uint64_t filter_index = (block_offset / kFilterBase);       // 需要创建的 filter 的数目
    assert(filter_index >= filter_offsets_.size());

    // filter_offset 保存每个 bloom filter 的起始偏移量
    while (filter_index > filter_offsets_.size()) {
        GenerateFilter();
    }
}


// 将 InternalKey 写入到 keys_ 中， 用 start_ 标识 key 的位置
// 将全部的数据写入完毕之后会调用 StartBlock 
void FilterBlockBuilder::AddKey(const Slice& key) {
    Slice k = key;
    start_.push_back(keys_.size());        // 新插入的key的起始位置
    keys_.append(k.data(), k.size());       // 直接append到keys中去
}


// 生成Filter, 每 2KB 生成一个 bloom 
void FilterBlockBuilder::GenerateFilter() {
    // 获得key，放到tmp_keys中
    const size_t num_keys = start_.size();

    if (num_keys == 0) {
        filter_offsets_.push_back(result_.size());
        return ;
    }

    start_.push_back(keys_.size());

    tmp_keys_.resize(num_keys);                 // 作为参数传入到生成 Filter 的函数


    // 将keys中的 InternalKey 取出来
    for (size_t i = 0; i < num_keys; i++) {
        const char* base = keys_.data() + start_[i];
        size_t len = start_[i+1] - start_[i];
        tmp_keys_[i] = Slice(base, len);
    }

    // 记录 Bloom Filter 结果的偏移量
    filter_offsets_.push_back(result_.size());

    policy_->CreateFilter(&tmp_keys_[0], static_cast<int>(num_keys), &result_);

    tmp_keys_.clear();
    keys_.clear();
    start_.clear();
}


// 完成最终的 MetaData 的写入，供读取的时候能够快速的定位
Slice FilterBlockBuilder::Finish() {
    if (!start_.empty()) {
        GenerateFilter();
    }

    // 将所有的偏移量写入到 result_ 的尾部, 也就是将很多的 index entry 写到尾巴
    size_t Size = result_.size();
    for (size_t i = 0; i < filter_offsets_.size(); i++) {
        PutFixed32(&result_, filter_offsets_[i]);
    }

    // 将过滤器的个数扔到尾部
    PutFixed32(&result_, Size);

    // 将Base也就是我们每2KB生成一个过滤器写入
    result_.push_back(kFilterBaselg);
    return Slice(result_);
}


FilterBlockReader::FilterBlockReader(const FilterPolicy* policy, const Slice& contents)
    : policy_(policy),
      data_(nullptr),
      offset_(nullptr),
      num_(0),
      base_lg_(0) {

//                                                              4B            1B
// filter_data1 filter_data2 ...  | filter_data_offsets | filter_data_size | base_lg

    size_t n = contents.size();
    if (n < 5) return;      // 1B[base_lg], 4B[start of offset]

    // 最后一个字节记录的是kFilterBaseLog
    base_lg_ = contents[n-1];

    // 4个字节记录 filter_data的大小， offset[s] 的起始位置
    uint32_t last_word = DecodeFixed32(contents.data() + n - 5);
    if (last_word > n - 5) return;

    data_ = contents.data();

    // filter data offset, 就是 offsets 的起始位置
    offset_ = data_ + last_word;

    // filter 的个数
    num_ = (n - 5 - last_word) / 4;
}


// 快速判断 key 是否存在于 block 中
bool FilterBlockReader::KeyMayMatch(uint64_t block_offset, const Slice& key) {
    // 位于哪一个 filter data
    uint64_t index = block_offset >> kFilterBaselg;
    // offset 就是 offset 数组的初始的位置
    if (index < num_) {
        uint32_t start = DecodeFixed32(offset_ + index * 4);
        uint32_t limit = DecodeFixed32(offset_ + index * 4 + 4);

        if (start <= limit && limit <= static_cast<size_t>(offset_ - data_)) {
            // 反解析出来对应的数据， bitmap
            Slice filter = Slice(data_ + start, limit - start);
            return policy_->KeyMayMatch(key, filter);
        } else if (start == limit) {
            // Empty filters do not match any keys
            return false;
        }
    }
    return true;
}

