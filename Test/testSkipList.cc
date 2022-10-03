#include "Comparator.h"
#include "Slice.h"
#include "arenaa.h"
#include "skipList.h"


using namespace xindb;
typedef uint64_t Key;

class ByteCom : public Comparator {
 public:
    ByteCom() {
        printf("Bytewise Comparator\n");
    }

    ~ByteCom() = default;

    int operator() (const Key& a, const Key& b) const {
        if (a > b) return 1;
        else if (a < b) return -1;
        return 0;
    }
    
    const char* Name() const override { return "kvdb.ByteComparator";}

    //call the function in Slcie
    int Compare(const Slice& a, const Slice& b) const override {
        printf("调用了 options->com->Compare..\n");
        return a.compare(b);
    }

    //find small between [start, limit)
    //将*start换成是一个很小的[start, limit)的字符串，字节顺序比较
    void FindShortestSeparator(std::string* start, const Slice& limit) const  {
        //Find the common prefix, 公共前缀
        size_t min_len = std::min(start->size(), limit.size());
        size_t index = 0;
        while (index != min_len && ((*start)[index] == limit[index])) {
            index++;
        }

        if (index >= min_len) {
            //Do nothing, 一个字符串是另一个的子集（缩减不了了）
        } else {
            //不是的话，将除了前缀之后的进行uint8 + 1
            uint8_t diff_byte = static_cast<uint8_t>((*start)[index]);

            if (diff_byte < static_cast<uint8_t>(0xff) && diff_byte + 1 < static_cast<uint8_t>(limit[index])) { 
                (*start)[index] ++;     //前缀之后的字节数目加上1
                start->resize(index+1);
                assert(Compare(*start, limit) < 0);
            }
        }
    }

    void FindShortSuccessor(std::string* key) const  {
        //Find first 
        size_t n = key->size();
        for (int i = 0; i < n; i++) {
            const uint8_t byte = (*key)[i];
            if (byte != static_cast<uint8_t>(0xff)) {
                (*key)[i] = byte + 1;
                key->resize(i + 1);
                return;
            }
        }
    }
};


ByteCom com1;   
using Table = SkipList<const Key, ByteCom>;

Arena arena_;
Table table(com1, &arena_);


void testSkip() {
    printf("begin test insert & contains...\n");
    int count = 0;
    int grade = 0;
    for (int i = 50; i <= 150; i++) {
        table.Insert(i);
        count ++;
    }
    assert(count == 101);
    if (count == 101) grade++;
    // printf("Begin judge if contains..., the current count : %d\n", count);
    for (int i = 50; i <= 150; i++) {
        if (table.Contains(i)) count --;
    }
    assert(count == 0);
    if (count == 0) grade ++;
    // printf("The final count %d, should 0\n", count);

    int fins = 0;
    for (int i = 1; i <= 500; i++) {
        if (table.Contains(i) && !(i >= 50 && i <= 150)) {
            fins++;
        }
    }
    assert(fins == 0);
    if (fins == 0) grade++;

    printf("There are three tests, pass grade %d / 3\n", grade);
}


void testIterator() {
    printf("Begin running test iterator...\n");
  SkipList<const Key, ByteCom>::Iterator iter(&table);
  if (iter.Valid()) printf("error, init iter should not valid\n");
  int grade = 0;
  iter.Seek(99);
  assert(iter.Valid());
  const Key& tmpkey1 = iter.key();
  if (tmpkey1 == 99) grade++;
//   printf("The first key which after 99 : %d\n", tmpkey1);

  iter.SeekForFirst();
  const Key& tmpkey2 = iter.key();
//   printf("The first key : %d\n", tmpkey2);
  if (tmpkey2 == 50) grade++;
 
  iter.SeekForLast();
  const Key& tmpkey3 = iter.key();
//   printf("The last key : %d\n", tmpkey3);
  if (tmpkey3 == 150) grade++;
  
  printf("There are three tests, pass grade %d / 3\n", grade);

}



// int main() {
//     testSkip();
//     testIterator();

//     return 0;
// }