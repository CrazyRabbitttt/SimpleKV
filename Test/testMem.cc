#include "memTable.h"
#include "skipList.h"
#include "InterCom.h"
#include "Comparator.h"

#include <string.h>
#include <string>
#include <stdio.h>

using namespace xindb;

const ByteWiseComparator* bytecom = new ByteWiseComparator();
InternalKeyCom internalcom(bytecom);
MemTable mem(internalcom);


void func() {

    // seq, type, key, value

    printf("Should hit [25, 54]\n");

    for (int i = 25; i < 55; i++) {
        if (i == 44) {
            std::string tmp = std::to_string(i);
            std::string tmpvalue = tmp + "val" + std::to_string(i);
            mem.Add(i, kTypeDeletion, tmp, tmpvalue);
            continue;
        }
        std::string tmp = std::to_string(i);
        std::string tmpvalue = tmp + "val" + std::to_string(i);
        mem.Add(i, kTypeValue, tmp, tmpvalue);
    }

    for (int i = 0; i < 60; i++) {
        std::string key = std::to_string(i);
        LookUpKey tmpkey(key, i);
        Status status;
        std::string value;
        bool r = mem.Get(tmpkey, &value, &status);  
        if (!r) { 
            printf("Mem table Get Error, key:%2d, value : %s\n", i, value.c_str()); 
        } else {
            printf("Hit the key & value, key:%2d, value : %s\n", i, value.c_str());
        }
    }
}

// int main() {
//     func();

// }

