#ifndef XINDB_UTIL_MUTEX_H_
#define XINDB_UTIL_MUTEX_H_

#include <assert.h>
#include <mutex>
#include <condition_variable>

namespace xindb {
namespace port {

class CondVar;

// 对于 std::mutex 的简单的封装
class Mutex {
 public:
    Mutex()  = default;
    ~Mutex() = default;

    Mutex(const Mutex&) = delete;
    Mutex&operator=(const Mutex&) = delete;

    void Lock() { mutex_.lock(); }
    void Unlock() { mutex_.unlock(); } 

 private:
    friend class CondVar;
    std::mutex mutex_;   
};  

// Tiny wraps of std::condition_variable 

class CondVar {
 public:

   explicit CondVar(Mutex* mutex) 
      : mutex_(mutex) { assert(mutex != nullptr); }

   ~CondVar() = default;

   CondVar(const CondVar&) = delete;
   CondVar&operator=(const CondVar&) = delete;

   void Wait() {
      std::unique_lock<std::mutex> lock(mutex_->mutex_, std::adopt_lock);
      cond_.wait(lock);
      lock.release();
   }

   void Signal() { cond_.notify_one(); }
   void SignalAll() { cond_.notify_all(); }

 private:
   std::condition_variable cond_;
   Mutex* const mutex_;
}; 

}   // namesapce xindb::port 
}   // namespace xindb 

#endif