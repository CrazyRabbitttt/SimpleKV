


#include "block.h"

#include "Status.h"
#include "Comparator.h"
#include "format.h"
#include "Coding.h"
#include "iterator.h"


namespace xindb {


// 反解析出 restart 的个数
uint32_t Block::NumRestart() const {
    assert(size_ > sizeof(uint32_t));
    return DecodeFixed32(data_ + size_ - sizeof(uint32_t));             // 减去4B
}

// entry1 entry2 entry3 entry4 ....  restart1 restart2 restart3 nums
Block::Block(const BlockContents& contents)
    : data_(contents.data.data()),
    size_(contents.data.size()),
    owend_(contents.heap_allocated) {
        // 解析出来 block 的各个参数
        if (size_ < sizeof(uint32_t)) {
            size_ = 0;          // 数据根本就不对
        } else {
            size_t max_restart_nums = (size_ - sizeof(uint32_t)) / sizeof(uint32_t);
            if (max_restart_nums < NumRestart()) {
                // size 太小
            } else {
                // 去掉末尾存储size的数据 和 restart 数据的大小， restart[0] 的位置
                restart_offset_ = size_ - (1 + NumRestart()) * sizeof(uint32_t);
            }
        }
    }

Block::~Block() {
    if (owend_) {
        delete[] data_;
    }
}


// shared_len|non_shared_len|val_len|Key|Value
// 最终将 pointer 指向 Key 所在的位置
static inline const char* DecodeEntry(const char* p, const char* limit,
                                        uint32_t* shared, uint32_t* non_shared,
                                        uint32_t* value_length) 
{
    if (limit - p < 3) return nullptr;
    // 假定三个数都是 一字节就能够存储的
    // 转为指针的原因： 获得一个元素，指针算是标识位置的作用，【指针即是数组， 
    // 算是重新解释内存中的内容，解引用的解释按照指针类型
    *shared       = reinterpret_cast<const uint8_t*>(p)[0];         
    *non_shared   = reinterpret_cast<const uint8_t*>(p)[1];
    *value_length = reinterpret_cast<const uint8_t*>(p)[2];

    if ((*shared | *non_shared | *value_length) < 128) {
        // 三个数存储都用一字节存储
        p += 3;
    } else {
        if ((p = GetVarint32Ptr(p, limit, shared)) == nullptr) return nullptr;
        if ((p = GetVarint32Ptr(p, limit, non_shared)) == nullptr) return nullptr;
        if ((p = GetVarint32Ptr(p, limit, value_length)) == nullptr) return nullptr;
    }

    // 如果说剩下的数据不足以保存
    if (static_cast<uint32_t>(limit - p) < (*non_shared + *value_length)) {
        return nullptr;
    }

    // 指向 Non_shared Key
    return p;
}

                                        


class Block::Iter : public Iterator {

 public:
    Iter(const Comparator* comparator, const char* data, uint32_t restarts, uint32_t num_restarts) 
        : comparator_(comparator),
         data_(data),
         restarts_(restarts), 
         num_restarts_(num_restarts),
         current_(restarts_), 
         restart_index_(num_restarts_) 
        {
            assert(num_restarts_ > 0);
        }

    // 当前所在的
    bool Valid() const override { return current_ < restarts_; }

    Status status() const override { return status_; }

    Slice key() const override {
        assert(Valid());
        return key_;
    }

    Slice value() const override {
        assert(Valid());
        return value_;
    }


 private:
    const Comparator* const comparator_;    
    const char* data_;                        // 解除 block 内容
    uint32_t restarts_;                       // Restart数组的偏移,restart[0]所在位置
    uint32_t num_restarts_;                   // Restart数组的数目

    // current 就是当前在 data 中 当前entry 的偏移量
    uint32_t current_;
    uint32_t restart_index_;                  // current 所在的 restart 块
    std::string key_;
    Slice value_;
    Status status_;


    inline int Compare(const Slice& a, const Slice& b) const {
        return comparator_->Compare(a, b);
    }

 public:

    //  查找第一个 >= target 的 Entry
    void Seek(const Slice& target) override {
        uint32_t left  = 0;
        uint32_t right = num_restarts_ - 1;
        int current_key_compare = 0;

        while (left < right) {
            uint32_t mid = (left + right + 1) / 2;
            uint32_t region_offset = GetRestartPoint(mid);          // 获得指定 restart的初始的 entry位置

            // 获得了 entry 的起始位置，开始解析 Entry， 获得group 的起始位置的参数进行二分查找
            uint32_t shared, no_shared, value_length;
            const char* key_ptr = DecodeEntry(data_ + region_offset, data_ + restarts_, &shared,
                                                &no_shared, &value_length);
            // Entry 的第一个数据肯定是记录了全部的数据的
            if (key_ptr == nullptr || (shared != 0)) {
                fprintf(stderr, "Err! Entry 的第一个数据 shared 应该是0");
                return ;
            }
            Slice mid_key(key_ptr, no_shared);
            if (Compare(mid_key, target) < 0) {
                left = mid;
            } else {
                right = mid - 1;
            }
        }

        // ======= 目前已经是找到了合适的 Group 了，直接遍历就好了 【最多16个嘛】===========\.
        assert(current_key_compare == 0 || Valid());

        // 目前所在的 index 块是不是目标的，不是的话跳过去
        bool skip_seek = left == restart_index_ && current_key_compare < 0;
        if (!skip_seek) {
            SeekToRestartPoint(left);
        }


        // 前面已经将 restartindex 标注好了
        while (true) {
            if (!ParseNextKey()) {
                return;
            }
            if (Compare(key_, target) >= 0) return;
        }

    }

    // 查找第一条数据，index 标记为1就好了
    void SeekToFirst() override {
        SeekToRestartPoint(0);
        ParseNextKey();
    }


    void SeekToLast() override {
        SeekToRestartPoint(num_restarts_ - 1);
        //  一直解析next Entry |   直到下一个entry是边界才停止
        while (ParseNextKey() && NextEntryOffset() < restarts_) {}
    }

    void Next() override {
        assert(Valid());
        ParseNextKey();
    }


    // iter 指向的前一个数值
    void Prev() override {
        assert(Valid());

        // 要找到对应的 重启点的呀

        // 保存一下现在节点的位置，最后用于比较
        const uint32_t original = current_;
        while (GetRestartPoint(restart_index_) >= original) {
            if (restart_index_ == 0) {      
                // 找不到，一般找不到的都回归原位[一个non_valid的位置]
                current_ = restarts_;
                restart_index_ = num_restarts_;
                return ;
            }
            restart_index_ --;
        }

        // 目前找到了数值对应的重启点 【restart】
        SeekToRestartPoint(restart_index_);
        do {
            // 直到下一个entry是original才停下
        } while (ParseNextKey() && NextEntryOffset() < original);

    }



 private:
    void CorruptionError() {
        current_ = restarts_;
        restart_index_ = num_restarts_;
        status_ = Status::Corruption("bad entry in block, 起始的key 中 shared应该为0");
        key_.clear();
        value_.clear();
    }

    // 记录 restart_index_, 维护value初始值
    void SeekToRestartPoint(uint32_t index) {
        key_.clear();
        restart_index_ = index;
        
        // 找到 restart 对应的 entry位置
        uint32_t offset = GetRestartPoint(index);
        value_ = Slice(data_ + offset, 0);              // 辅助 ParseNextKey
    }

    // 传入 restart中的下标， 返回具体的偏移
    uint32_t GetRestartPoint(uint32_t index) {
        assert(index < num_restarts_);
        return DecodeFixed32(data_ + restarts_ + index * sizeof(uint32_t));
    }

    // 把 iter 指向下一个 entry
    bool ParseNextKey() {
        // 确定好 p， limit
        current_ = NextEntryOffset();
        const char* p = data_ + current_;
        const char* limit = data_ + restarts_;

        if (p >= limit) {
            // invalid 
            current_ = restarts_;
            restart_index_ = num_restarts_;
            return false;
        }

        // Decode next entry 
        uint32_t shared, non_shared, value_length;
        p = DecodeEntry(p, limit, &shared, &non_shared, &value_length);
        if (p == nullptr || key_.size() < shared) {
            CorruptionError();
            return false;
        } else {
            key_.resize(shared);
            key_.append(p, non_shared);
            value_ = Slice(p + non_shared, value_length);
            while (restart_index_ + 1 < num_restarts_ && 
                    GetRestartPoint(restart_index_ + 1) < current_)         // ???? 为什么会出现这种情况？
            {
                restart_index_++;                                           // 读取完了本 entry 的数据，转移 restart index
            }
            return true;
        }
    }

    // 下一个 entry 的位置，通过这一个entry的 value 的末尾就能够确认
    uint32_t NextEntryOffset() const {
        return (value_.data() + value_.size()) - data_;
    }

};



Iterator* Block::NewIterator(const Comparator* comparator) {
    const uint32_t num_restarts = NumRestart();
    return new Iter(comparator, data_, restart_offset_, num_restarts);
}


}   // namespace xindb 
