#ifndef XINDB_UTIL_SLICE_H
#define XINDB_UTIL_SLICE_H

#include <string.h>
#include <string>
#include <assert.h>

namespace xindb {

class Slice {
 public:

    Slice()                                 // default 
        : data_(""), size_(0) {}

    Slice(const char* data, size_t len) 
        : data_(data), size_(len) {}

    Slice(const std::string& s)
        : data_(s.data()), size_(s.size()) {}

    Slice(const char* data) 
        : data_(data), size_(strlen(data)) {}

    Slice(const Slice& s) = default;
    Slice& operator=(const Slice& s) = default;

    const char* data() const { return data_; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    char operator[](size_t n) const {
        assert(n < size());
        return data_[n];
    }

    void clear() {
        data_ = "";
        size_ = 0;
    }

    // 删除前n个字符？
    void remove_prefix(size_t n) {
        assert(n <= size());
        data_ += n;
        size_ -= n;
    }

    // Use move decrease the consume of copy 
    std::string ToString() const {
        return std::move(std::string(data_, size_));
    }

    int compare(const Slice& b) const;

    // 是否是以Slice b为前缀的？
    bool start_with(const Slice& b) const {
        return ((size_ > b.size_) && (memcmp(data_, b.data(), b.size()) == 0));
    }

 private:
    const char* data_;
    size_t size_;

};

inline bool operator==(const Slice& x, const Slice& y) {
    return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline int Slice::compare(const Slice& b) const {
    const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
    int res = memcmp(data_, b.data_, min_len);
    if (res == 0) {     // 如果是前缀，len打的返回
        if (size_ < b.size_) return -1;
        else if (size_ > b.size_) return +1;
    }
    return res;
}

}   // namespace xindb 

#endif 