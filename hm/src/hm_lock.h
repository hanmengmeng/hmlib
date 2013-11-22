#ifndef HM_LOCK_H_H
#define HM_LOCK_H_H

#include <windows.h>

#include "hm_common.h"

namespace hm
{

class ThreadLock
{
public:
    ThreadLock();
    ~ThreadLock();

    bool Try();

    // Take the lock, blocking until it is available if necessary.
    void Lock();

    // Release the lock.  This must only be called by the lock's holder: after
    // a successful call to Try, or a call to Lock.
    void Unlock();

    // Return the native underlying lock.
    CRITICAL_SECTION* os_lock() { return &os_lock_; }

private:
    CRITICAL_SECTION os_lock_;

    DISALLOW_COPY_AND_ASSIGN(ThreadLock);
};

}

#endif