#include "hm_thread.h"
#include <process.h>

unsigned int __stdcall Thread::ThreadProc(void *param)
{
    Thread *pt = (Thread*)param;
    return pt->Run();
}

Thread::Thread()
{
    mThread = NULL;
    mThreadId = 0;
}

Thread::~Thread()
{
    if (IsRunning())
    {
        Stop();
    }
}

BOOL Thread::Start( DWORD delay /*= 0*/ )
{
    if(NULL != mThread)
    {
        return FALSE;
    }
    mThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, (unsigned int*)mThreadId);
    return mThread != NULL;
}

BOOL Thread::Stop()
{
    if(!IsRunning())
    {
        if(NULL != mThread)
        {
            CloseHandle(mThread);
            mThread = NULL;
        }
        mThreadId = 0;
        return TRUE;
    }

    if(StopRequest())
    {
        if(Join(1000))
        {
            return TRUE;
        }
    }

    _endthreadex(0);
    CloseHandle(mThread);
    mThread = NULL;
    mThreadId = 0;

    return TRUE;
}

BOOL Thread::Join( DWORD waitTime /*= INFINITE*/ )
{
    if(NULL == mThread)
    {
        return FALSE;
    }

    DWORD dwRet = WaitForSingleObject(mThread, waitTime);
    if(WAIT_OBJECT_0 != dwRet)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL Thread::IsRunning()
{
    return WaitForSingleObject(mThread, 0) == WAIT_TIMEOUT;
}

WorkThread::WorkThread()
{
    mQueueWorksLock = ::CreateMutex(NULL, FALSE, NULL);
    mWorksEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    mExitEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    mCurrentWorkId = INVALID_WORK_ID;
}

WorkThread::~WorkThread()
{
    CloseHandle(mExitEvent);
    CloseHandle(mWorksEvent);
    CloseHandle(mQueueWorksLock);
}

BOOL WorkThread::Add( const struct WorkThreadParam &wtp )
{
    WaitForSingleObject(mQueueWorksLock, INFINITE);
    if (wtp.flag & WORK_EXCLUSIVE)
    {
        if (mCurrentWorkId == wtp.id)
        {
            ReleaseMutex(mQueueWorksLock);
            return FALSE;
        }
        WorkQueue::iterator it;
        while (it != mQueueWorks.end())
        {
            if (it->id == wtp.id)
            {
                break;
            }
        }
        if (it != mQueueWorks.end()) // Already in queue
        {
            ReleaseMutex(mQueueWorksLock);
            return FALSE;
        }
    }

    mQueueWorks.push_back(wtp);
    ReleaseMutex(mQueueWorksLock);
    SetEvent(mWorksEvent);
    return TRUE;
}

size_t WorkThread::GetCount()
{
    return mQueueWorks.size();
}

int WorkThread::Run()
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
                    struct WorkThreadParam wtp = mQueueWorks.front();
                    mQueueWorks.pop_front();
                    mCurrentWorkId = wtp.id;
                    ReleaseMutex(mQueueWorksLock);

                    wtp.doWork(wtp.id, wtp.extraData);

                    WaitForSingleObject(mQueueWorksLock, INFINITE);
                    mCurrentWorkId = INVALID_WORK_ID;
                    ReleaseMutex(mQueueWorksLock);
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

BOOL WorkThread::StopRequest()
{
    ::SetEvent(mExitEvent);
    return TRUE;
}

int WorkThread::IsRunning( int workId )
{
    return workId == mCurrentWorkId;
}
