#ifndef HM_THREAD_POOL_H_H
#define HM_THREAD_POOL_H_H

#include <deque>
#include <vector>

#include "hm_thread.h"

class WorkAdapterBase
{
public:
    virtual ~WorkAdapterBase(){}
    virtual bool Execute() = 0;
};

template <typename Functor>
class WorkAdapter : public WorkAdapterBase
{};

template <typename R, typename T>
class WorkAdapter<R(T::*)()> : public WorkAdapterBase {
public:
    typedef R (RunType)(T*);

    explicit WorkAdapter(T* obj, R(T::*method)()) {
        object_ = obj;
        method_ = method;
    }

    virtual bool Execute(){
        object_->*method_();
    }

private:
    T *object_;
    R(T::*method_)();
};

template <typename R, typename T, typename A1>
class WorkAdapter<R(T::*)(A1)> : public WorkAdapterBase {
public:
    typedef R (RunType)(T*, A1);

    explicit WorkAdapter(T* obj, R(T::*method)(A1), A1 a1) {
        object_ = obj;
        method_ = method;
        a1_ = a1;
    }

    virtual bool Execute(){
        object_->*method_(a1);
    }

private:
    T *object_;
    R(T::*method_)(A1);
    A1 a1_;
};

template <typename R, typename T, typename A1, typename A2>
class WorkAdapter<R(T::*)(A1, A2)> : public WorkAdapterBase {
public:
    typedef R (RunType)(T*, A1, A2);

    explicit WorkAdapter(T* obj, R(T::*method)(A1), A1 a1, A2 a2) {
        object_ = obj;
        method_ = method;
        a1_ = a1;
        a2_ = a2;
    }

    virtual bool Execute(){
        object_->*method_(a1, a2);
    }

private:
    T *object_;
    R(T::*method_)(A1);
    A1 a1_;
    A2 a2_;
};

template <typename R, typename T, typename A1, typename A2, typename A3>
class WorkAdapter<R(T::*)(A1, A2, A3)> : public WorkAdapterBase {
public:
    typedef R (RunType)(T*, A1, A2, A3);

    explicit WorkAdapter(T* obj, R(T::*method)(A1), A1 a1, A2 a2, A3 a3) {
        object_ = obj;
        method_ = method;
        a1_ = a1;
        a2_ = a2;
        a3_ = a3;
    }

    virtual bool Execute(){
        object_->*method_(a1, a2, a3);
    }

private:
    T *object_;
    R(T::*method_)(A1);
    A1 a1_;
    A2 a2_;
    A3 a3_;
};

template <typename R, typename T, typename A1, typename A2, typename A3, typename A4>
class WorkAdapter<R(T::*)(A1, A2, A3, A4)> : public WorkAdapterBase {
public:
    typedef R (RunType)(T*, A1, A2, A3, A4);

    explicit WorkAdapter(T* obj, R(T::*method)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4) {
        object_ = obj;
        method_ = method;
        a1_ = a1;
        a2_ = a2;
        a3_ = a3;
        a4_ = a4;
    }

    virtual bool Execute(){
        (object_->*method_)(a1_, a2_, a3_, a4_);
        return true;
    }

private:
    T *object_;
    R(T::*method_)(A1, A2, A3, A4);
    A1 a1_;
    A2 a2_;
    A3 a3_;
    A4 a4_;
};

template <typename R, typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
class WorkAdapter<R(T::*)(A1, A2, A3, A4, A5)> : public WorkAdapterBase {
public:
    typedef R (RunType)(T*, A1, A2, A3, A4, A5);

    explicit WorkAdapter(T* obj, R(T::*method)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
        object_ = obj;
        method_ = method;
        a1_ = a1;
        a2_ = a2;
        a3_ = a3;
        a4_ = a4;
        a5_ = a5;
    }

    virtual bool Execute(){
        object_->*method_(a1_, a2_, a3_, a4_, a5_);
    }

private:
    T *object_;
    R(T::*method_)(A1);
    A1 a1_;
    A2 a2_;
    A3 a3_;
    A4 a4_;
    A5 a5_;
};

template <typename T>
struct FunctorTraits {
    typedef WorkAdapter<T> RunnableType;
    typedef typename RunnableType::RunType RunType;
};

// template 
template <typename T, typename O>
typename FunctorTraits<T>::RunnableType *CreateWorkAdapter(const T& t, O o) {
    return new WorkAdapter<T>(o, t);
}

template <typename T, typename O, typename A1>
typename FunctorTraits<T>::RunnableType *CreateWorkAdapter(const T& t, O o, A1 a1) {
    return new WorkAdapter<T>(o, t, a1);
}

template <typename T, typename O, typename A1, typename A2>
typename FunctorTraits<T>::RunnableType *CreateWorkAdapter(const T& t, O o, A1 a1, A2 a2) {
    return new WorkAdapter<T>(o, t, a1, a2);
}

template <typename T, typename O, typename A1, typename A2, typename A3>
typename FunctorTraits<T>::RunnableType *CreateWorkAdapter(const T& t, O o, A1 a1, A2 a2, A3 a3) {
    return new WorkAdapter<T>(o, t, a1, a2, a3);
}

template <typename T, typename O, typename A1, typename A2, typename A3, typename A4>
typename FunctorTraits<T>::RunnableType *CreateWorkAdapter(const T& t, O o, A1 a1, A2 a2, A3 a3, A4 a4) {
    return new WorkAdapter<T>(o, t, a1, a2, a3, a4);
}

template <typename T, typename O, typename A1, typename A2, typename A3, typename A4, typename A5>
typename FunctorTraits<T>::RunnableType *CreateWorkAdapter(const T& t, O o, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    return new WorkAdapter<T>(o, t, a1, a2, a3, a4, a5);
}

typedef std::deque<WorkAdapterBase*> WorkItemQueue;

class WmWorkThread : public Thread
{
public:
    WmWorkThread();
    ~WmWorkThread();

    bool AddWork(WorkAdapterBase *wab);
    size_t GetWorksNumber();

protected:
    virtual int Run();
    virtual BOOL StopRequest();

protected:
    WorkItemQueue mQueueWorks;
    HANDLE mQueueWorksLock;
    HANDLE mWorksEvent;
    HANDLE mExitEvent;
};

class WmWorkThreadPool
{
public:
    WmWorkThreadPool();
    ~WmWorkThreadPool();

    // Start all threads
    void Start();

    // Stop all threads
    void Stop();

    // Set the max number of threads
    void SetThreadCapacity(size_t count);

    // Add work to thread pool,
    // pool will pick the most idle thread to execute it
    bool AddWork(WorkAdapterBase *wab);

private:
    WmWorkThread *GetIdleThread();

protected:
    std::vector<WmWorkThread*> thread_pool_;
    size_t max_thread_count_;
    bool thread_start_flag_;
};

#endif
