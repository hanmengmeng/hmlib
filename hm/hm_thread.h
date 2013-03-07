#ifndef HM_THREAD_H
#define HM_THREAD_H

#include <deque>

#include <Windows.h>

class Thread
{
public:
    Thread();
    virtual ~Thread();

    BOOL Start(DWORD delay = 0);
    BOOL Stop();
    BOOL Join(DWORD waitTime = INFINITE);
    BOOL IsRunning();

protected:
    virtual int Run() = 0;
    virtual BOOL StopRequest() = 0;

private:
    static unsigned int __stdcall ThreadProc(void *param);

private:
    HANDLE mThread;
    DWORD mThreadId;
};

typedef int *fnWorkFunc(int jobId, void *extraData);
struct WorkThreadParam
{
    int id;
    void *extraData;
    short flag;
    fnWorkFunc doWork;
};
typedef std::deque<struct WorkThreadParam> WorkQueue;

enum // Work flag
{
    WORK_EXCLUSIVE = 1 << 0,
    WORK_MULTIPLE = 1 << 1,
};
#define INVALID_WORK_ID ~0

class WorkThread : public Thread
{
public:
    WorkThread();
    ~WorkThread();

    BOOL Add(const struct WorkThreadParam &wtp);
    size_t GetCount();
    int IsRunning(int id);

protected:
    virtual int Run();
    virtual BOOL StopRequest();

private:
    WorkQueue mQueueWorks;
    HANDLE mQueueWorksLock;
    HANDLE mWorksEvent;
    HANDLE mExitEvent;
    size_t mCurrentWorkId;
};

#endif
