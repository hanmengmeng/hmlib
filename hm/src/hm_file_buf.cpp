#include "hm_file_buf.h"
#include <assert.h>
#include <Windows.h>
#include <tchar.h>

namespace hm
{

FileBuf::FileBuf( const wchar_t *path, int mode )
{
    Init(path, mode);
}

FileBuf::FileBuf( const wchar_t *path, int mode, size_t bufSize )
{
    Init(path, mode);
    mBuf = new char[bufSize];
    mBufSize = bufSize;
    mCurrPos = 0;
}

FileBuf::FileBuf()
{
    Init(NULL, 0);
}

FileBuf::~FileBuf()
{
    Close();
    if (NULL != mBuf)
    {
        delete []mBuf;
    }
}

size_t FileBuf::Write( void *buf, size_t len )
{
    if (NULL == mFd)
    {
        return 0;
    }

    // Space left is not enough
    size_t spaceLeft = mBufSize - mCurrPos;
    if (len > spaceLeft)
    {
        if (Flush(buf, len))
        {
            return len;
        }
        else
        {
            return 0;
        }
    }

    // Add to buffer
    memcpy(mBuf+mCurrPos, buf, len);
    mCurrPos += len;
    return len;
}

size_t FileBuf::Read( void *buf, size_t len )
{
    if (NULL == mFd)
    {
        return 0;
    }
    return fread(buf, 1, len, mFd);
}

hm_long_64 FileBuf::GetFileSize()
{
    return mFileSize;
}

bool FileBuf::Flush(void *buf, size_t len)
{
    if (NULL == mFd)
    {
        return false;
    }

    if (mCurrPos > 0) // Flush the buffer to disk
    {
        size_t writeBufLen = 0;
        writeBufLen = fwrite(mBuf, 1, mCurrPos, mFd);
        mCurrPos -= writeBufLen;
        if (mCurrPos != 0)
        {
            return false;
        }
    }
    if (NULL != buf && len > 0)
    {
        return fwrite(buf, 1, len, mFd) == len;
    }
    else
    {
        return true;
    }
}

bool FileBuf::Reserve( void **buf, size_t len )
{
    size_t spaceLeft = mBufSize - mCurrPos;
    *buf = NULL;

    if (len > mBufSize)
    {
        return false;
    }

    if (spaceLeft <= len)
    {
        if (!Flush(NULL, 0)) // Only flush the buffer to disk
        {
            return false;
        }
    }

    *buf = (mBuf + mCurrPos);
    mCurrPos += len;
    return true;
}

void FileBuf::Init( const wchar_t *path, int mode )
{
    // Open the file and get the file size
    if (NULL != path && Open(path, mode))
    {
        fpos_t pos;
        if (0 == fgetpos(mFd, &pos))
        {
            if (0 == fseek(mFd, 0L, SEEK_END))
            {
                fpos_t sizepos;
                if (0 == fgetpos(mFd, &sizepos))
                {
                    mFileSize = sizepos;
                }
                else
                {
                    mFileSize = 0;
                }
            }
            else
            {
                mFileSize = 0;
            }
            assert(fseek(mFd, 0L, SEEK_SET) == 0);
            assert(fsetpos(mFd, &pos) == 0);
        }
    }
    else
    {
        mFileSize = 0;
    }
    mBuf = NULL;
    mBufSize = 0;
    mCurrPos = 0;
}

bool FileBuf::Open( const wchar_t *path, int mode )
{
    if ((mode & FILE_MODE_WRITE) && (mode & FILE_MODE_READ))
    {
        if (_tfopen_s(&mFd, path, _T("w+b")) != 0)
        {
            mFd = NULL;
        }
    }
    else if ((mode & FILE_MODE_APPEND) && (mode & FILE_MODE_READ))
    {
        if (_tfopen_s(&mFd, path, _T("a+b")) != 0)
        {
            mFd = NULL;
        }
    }
    else if (mode & FILE_MODE_APPEND)
    {
        if (_tfopen_s(&mFd, path, _T("ab")) != 0)
        {
            mFd = NULL;
        }
    }
    else if (mode & FILE_MODE_WRITE)
    {
        if (_tfopen_s(&mFd, path, _T("wb")) != 0)
        {
            mFd = NULL;
        }
    }
    else if (mode & FILE_MODE_READ)
    {
        if (_tfopen_s(&mFd, path, _T("rb")) != 0)
        {
            mFd = NULL;
        }
    }
    else
    {
        assert(0); // Unknown file mode
    }
    return mFd != NULL;
}

void FileBuf::Close()
{
    if (NULL != mFd)
    {
        Flush(NULL, 0);
        fclose(mFd);
        mFd = NULL;
    }
}


bool HashFileBuf::HashResult( unsigned char *hashResult, size_t len )
{
    if (len >= HASH_SHA1_LEN)
    {
        mHash.Final(hashResult);
        return true;
    }
    else
    {
        return false;
    }
}

bool HashFileBuf::Flush(void *buf, size_t len)
{
    if (mCurrPos > 0)
    {
        mHash.Update(mBuf, mCurrPos);
    }
    mHash.Update(buf, len);
    return FileBuf::Flush(buf, len);
}

size_t HashFileBuf::Read( void *buf, size_t len )
{
    size_t readLen = FileBuf::Read(buf, len);
    mHash.Update(buf, readLen);
    return readLen;
}

HashFileBuf::HashFileBuf( const wchar_t *path, int mode )
    :FileBuf(path, mode)
{
    assert(mode == FILE_MODE_READ || mode == FILE_MODE_WRITE);
}

HashFileBuf::HashFileBuf( const wchar_t *path, int mode, size_t bufSize )
    :FileBuf(path, mode, bufSize)
{
    assert(mode == FILE_MODE_READ || mode == FILE_MODE_WRITE);
}

HashFileBuf::HashFileBuf()
{

}

HashFileBuf::~HashFileBuf()
{
}

void HashFileBuf::HashReset()
{
    mHash.Reset();
}

}
