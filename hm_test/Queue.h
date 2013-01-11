#ifndef QUEUE_H_
#define QUEUE_H_

// implement using linked list
template <class T>
class Queue
{
public:
    Queue()
    {
        mBegin = NULL;
        mEnd = NULL;
        mSize = 0;
    }
    ~Queue()
    {
        while (GetSize() > 0)
        {
            PopFront();
        }
    }

    void PushBack(T node)
    {
        T *n = new T(node);
        struct tn *p = new struct tn;
        p->value = n;
        p->next = NULL;

        if (mBegin == NULL)
        {
            mBegin = p;
            mEnd = p;
        }
        else
        {
            mEnd->next = p;
            mEnd = p;
        }
        mSize++;
    }

    T PopFront()
    {
        if (NULL == mBegin)
        {
            return NULL;
        }
        struct tn *pTn = mBegin;
        T t = *(pTn->value);
        mBegin = mBegin->next;
        delete pTn->value;
        delete pTn;
        mSize--;

        return t;
    }

    size_t GetSize()
    {
        return mSize;
    }

    bool IsEmpty()
    {
        return mSize == 0;
    }

private:
    struct tn
    {
        T *value;
        struct tn *next;
    };

    struct tn *mBegin;
    struct tn *mEnd;
    size_t mSize;
};

// Implement using array
template <class T>
class QueueA
{
public:
    QueueA()
    {
        mBuffer = NULL;
        mBegin = 0;
        mEnd = 0;
        mSize = 0;
    }

    ~QueueA()
    {
        if (NULL != mBuffer)
        {
            delete []mBuffer;
        }
    }

    void PushBack(T t)
    {
        if (NULL == mBuffer)
        {
            mSize = 16;
            mBuffer = new T[mSize];
            mBegin = 0;
            mEnd = 0;
        }
        else if (mEnd == mSize - 1)
        {
            mSize *= 2;
            T *t = new T[mSize];
            memcpy(t, mBuffer+mBegin, (mEnd-mBegin)*sizeof(T));
            mEnd = mEnd - mBegin;
            mBegin = 0;
            delete []mBuffer;
            mBuffer = t;
        }
        mBuffer[mEnd++] = t;
    }

    T PopFront()
    {
        return mBuffer[mBegin++];
    }

    size_t GetSize()
    {
        return mEnd - mBegin;
    }

    bool IsEmpty()
    {
        return (mEnd - mBegin) == 0;
    }

private:
    T *mBuffer;
    size_t mBegin;
    size_t mEnd;
    size_t mSize;
};

#endif
