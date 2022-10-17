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
    // 3. Type降序
    printf("Debug:InternalKey Comapre: key[%s], value[%s]\n", akey.data(), bkey.data());
    // printf("The user's comparator name is :%s\n", user_com_->Name());

    printf("...\n");
    // return -1;
    // int res = user_com_->Compare(ExtractUserKey(akey), ExtractUserKey(bkey));
    Slice a = ExtractUserKey(akey), b = ExtractUserKey(bkey);
    int res = a.compare(b);
    if (res == 0) {
        // SeqNum 进行比较
        const uint64_t anum = DecodeFixed64(akey.data() + akey.size() - 8);
        const uint64_t bnum = DecodeFixed32(bkey.data() + bkey.size() - 8);

        if (anum > bnum) res = -1;
        else if (anum < bnum) res = +1;
    }
    return res;
}


