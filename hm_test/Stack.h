#ifndef STACK_H_
#define STACK_H_

#include <assert.h>

template <class T>
class Stack
{
public:
    Stack()
    {
        mTop = NULL;
        mSize = 0;
    }

    ~Stack()
    {
        while (!IsEmpty())
        {
            PopUp();
        }
    }

    void PushBack(T t)
    {
        struct Node *n = new struct Node;
        n->value = t;
        n->next = NULL;
        n->pre = NULL;
        if (0 == mSize)
        {
            mTop = n;
        }
        else
        {
            n->pre = mTop;
            mTop->next = n;
            mTop = n;
        }
        mSize++;
    }

    T PopUp()
    {
        assert(mSize > 0);
        if (mSize > 0)
        {
            struct Node *top = mTop;
            struct Node *pre = mTop->pre;
            if (NULL != pre)
            {
                pre->next = NULL;
            }
            T ret = top->value;
            delete top;
            mTop = pre;
            mSize--;
            return ret;
        }
        else
        {
            return NULL;
        }
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
    struct Node
    {
        T value;
        struct Node *pre;
        struct Node *next;
    };
    struct Node *mTop;
    size_t mSize;
};


template <class T>
class StackA
{
public:
    StackA()
    {
        mBuffer = NULL;
        mSize = 0;
        mTop = 0;
    }

    ~StackA()
    {
        if (NULL != mTop)
        {
            delete []mTop;
        }
    }

    void PushBack(T t)
    {
        if (NULL == mBuffer)
        {
            mSize = 16;
            mBuffer = new T[mSize];
        }
        else if (mTop + 1 == mSize)
        {
            mSize *= 2;
            T *t = new T[mSize];
            memcpy(t, mBuffer, (mTop+1)*sizeof(T));
            delete []mBuffer;
            mBuffer = t;
        }
        mBuffer[mTop++] = t;
    }

    T PopUp()
    {
        if (mTop > 1 && mBuffer != NULL)
        {
            return mBuffer[--mTop];
        }
        else
        {
            return NULL;
        }
    }

    size_t GetSize()
    {
        return mTop;
    }

    bool IsEmpty()
    {
        return mTop == 0;
    }

private:
    T *mBuffer;
    size_t mSize;
    size_t mTop;
};

#endif
