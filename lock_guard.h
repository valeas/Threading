#ifndef LOCK_GUARD_H
#define LOCK_GUARD_H

#endif // LOCK_GUARD_H

#include <mutex>

namespace myLock {

template <class Mutex>
class lock_guard {
 public:
    typedef Mutex mutex_type; //define the mutex type, can be changed to lockable witout much issue
    explicit lock_guard(mutex_type& mut): locker(mut) {
       locker.lock();
    }

    lock_guard(mutex_type& mut, std::adopt_lock_t): locker(mut)
    { } // calling thread owns mutex, does not need it

    //free mutex lock on destructor call
    ~lock_guard(){
        locker.unlock();
    }

    //prevent some errors that could make the lock useless
    //also, prevent simple methods to reference/copy it :(
    lock_guard(const lock_guard&) = delete;
    lock_guard& operator=(const lock_guard&) = delete;

private:
    //this & took me way too much time to find that I was missing...
    mutex_type& locker;

};


}
