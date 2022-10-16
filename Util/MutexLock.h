#ifndef XINDB_UTIL_MUTEXLOCK_H_
#define XINDB_UTIL_MUTEXLOCK_H_

#include "Mutex.h"

namespace xindb {

class MutexLock {
 public:    
    explicit MutexLock(Mutex* mutex)
        : mutex_(mutex) {
            this->mutex_->Lock();
        }

    ~MutexLock() {
        this->mutex_->Unlock();
    }

    MutexLock(const MutexLock&) = delete;
    MutexLock&operator=(const MutexLock&) = delete;

 private:
    // Mutex* const mutex_;
    port::Mutex* const mutex_;  

};  


}   // namespace xindb 

#endif