#include "hm_lock.h"

namespace hm
{

ThreadLock::ThreadLock()
{
    // The second parameter is the spin count, for short-held locks it avoid the
    // contending thread from going to sleep which helps performance greatly.
    ::InitializeCriticalSectionAndSpinCount(&os_lock_, 2000);
}

ThreadLock::~ThreadLock()
{
    ::DeleteCriticalSection(&os_lock_);
}

bool ThreadLock::Try()
{
    if (::TryEnterCriticalSection(&os_lock_) != FALSE) {
        return true;
    }
    return false;
}

void ThreadLock::Lock()
{
    ::EnterCriticalSection(&os_lock_);
}

void ThreadLock::Unlock()
{
    ::LeaveCriticalSection(&os_lock_);
}

}


