#include "InterCom.h"
#include "Coding.h"

using namespace xindb;


const char* InternalKeyCom::Name() const {
    return "xindb.internalkeyCom";
}


int InternalKeyCom::Compare(const Slice& akey, const Slice& bkey) const {
    // Order by:
    // 1. UserKey升序
    // 2. Seq降序
    // 3. Type降序·
    // int res = user_com_->Compare(ExtractUserKey(akey), ExtractUserKey(bkey));
    Slice a = ExtractUserKey(akey), b = ExtractUserKey(bkey);
    int res = a.compare(b);
    if (res == 0) {
        // SeqNum 进行比较, 降序的比较，seq比较大【最新的】的数值优先
        const uint64_t anum = DecodeFixed64(akey.data() + akey.size() - 8);
        const uint64_t bnum = DecodeFixed32(bkey.data() + bkey.size() - 8);

        if (anum > bnum) res = -1;
        else if (anum < bnum) res = +1;
    }
    return res;
}


