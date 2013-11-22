#include "hm_thread_pool.h"

WmWorkThread::WmWorkThread()
{
    mQueueWorksLock = ::CreateMutex(NULL, FALSE, NULL);
    mWorksEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    mExitEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

WmWorkThread::~WmWorkThread()
{
    WaitForSingleObject(mQueueWorksLock, INFINITE);
    for (size_t i = 0; i < mQueueWorks.size(); i++) {
        delete mQueueWorks.at(i);
    }
    ReleaseMutex(mQueueWorksLock);

    CloseHandle(mExitEvent);
    CloseHandle(mWorksEvent);
    CloseHandle(mQueueWorksLock);
}

bool WmWorkThread::AddWork( WorkAdapterBase *wab )
{
    WaitForSingleObject(mQueueWorksLock, INFINITE);

    mQueueWorks.push_back(wab);
    ReleaseMutex(mQueueWorksLock);

    SetEvent(mWorksEvent);
    return true;
}

size_t WmWorkThread::GetWorksNumber()
{
    return mQueueWorks.size();
}

int WmWorkThread::Run()
{
    BOOL bExit = FALSE;
    DWORD dwWaitReturn;
    HANDLE waitEvents[] = { mExitEvent, mWorksEvent };
    while (TRUE)
    {
        dwWaitReturn = WaitForMultipleObjects(sizeof(waitEvents)/sizeof(waitEvents[0]), waitEvents, FALSE, INFINITE);
        switch (dwWaitReturn)
        {
        case WAIT_TIMEOUT:
            break;
        case WAIT_FAILED:
            break;
        case WAIT_OBJECT_0: // Exit
            bExit = TRUE;
            break;
        case WAIT_OBJECT_0 + 1:
            {
                while (!mQueueWorks.empty())
                {
                    WaitForSingleObject(mQueueWorksLock, INFINITE);
                    WorkAdapterBase *wab = mQueueWorks.front();
                    mQueueWorks.pop_front();
                    ReleaseMutex(mQueueWorksLock);

                    wab->Execute();
                    delete wab;
                }
            }
            break;
        default:
            break;
        }

        if (bExit)
        {
            break;
        }
    }

    return 0;
}

BOOL WmWorkThread::StopRequest()
{
    ::SetEvent(mExitEvent);
    return TRUE;
}

WmWorkThreadPool::WmWorkThreadPool()
{
    max_thread_count_ = 5;
    thread_pool_.push_back(new WmWorkThread());
    thread_pool_.push_back(new WmWorkThread());
    thread_start_flag_ = false;
}

WmWorkThreadPool::~WmWorkThreadPool()
{
    for (size_t i = 0; i < thread_pool_.size(); i++)
    {
        delete thread_pool_.at(i);
    }
}

void WmWorkThreadPool::SetThreadCapacity( size_t count )
{
    if (count > max_thread_count_) {
        max_thread_count_ = count;
    }
}

bool WmWorkThreadPool::AddWork( WorkAdapterBase *wab )
{
    if (!thread_start_flag_) {
        Start();
    }
    WmWorkThread *thread = GetIdleThread();
    if (NULL == thread) {
        return false;
    }
    return thread->AddWork(wab);
}

WmWorkThread * WmWorkThreadPool::GetIdleThread()
{
    WmWorkThread *most_idle = thread_pool_.at(0);

    // Find the most idle thread in current thread pool
    for (size_t i = 1; i < thread_pool_.size(); i++)
    {
        if (most_idle->GetWorksNumber() > thread_pool_.at(i)->GetWorksNumber())
        {
            most_idle = thread_pool_.at(i);
        }
    }

    if (most_idle->GetWorksNumber() > 0 && thread_pool_.size() < max_thread_count_)
    {
        most_idle = new WmWorkThread();
        most_idle->Start();
        thread_pool_.push_back(most_idle);
    }
    return most_idle;
}

void WmWorkThreadPool::Start()
{
    for (size_t i = 0; i < thread_pool_.size(); i++)
    {
        if (!thread_pool_.at(i)->IsRunning()) {
            thread_pool_.at(i)->Start();
        }
    }
    thread_start_flag_ = true;
}

void WmWorkThreadPool::Stop()
{
    for (size_t i = 0; i < thread_pool_.size(); i++)
    {
        thread_pool_.at(i)->Stop();
    }
    thread_start_flag_ = false;
}
