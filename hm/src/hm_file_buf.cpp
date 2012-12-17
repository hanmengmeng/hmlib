#include "hm_file_buf.h"
#include <assert.h>
#include <Windows.h>
#include <tchar.h>

namespace hm
{

FileBuf::FileBuf( const hm_string &path, int mode )
{
    if (mode & FILE_MODE_APPEND)
    {
        mFd = _tfopen(path.c_str(), _T("a+b"));
    }
    else if (mode & FILE_MODE_WRITE)
    {
        mFd = _tfopen(path.c_str(), _T("w+b"));
    }
    else if (mode & FILE_MODE_READ)
    {
        mFd = _tfopen(path.c_str(), _T("rb"));
    }
    else
    {
        assert(0); // Unknown file mode
    }

    // Get the file size
    if (NULL != mFd)
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
}

FileBuf::~FileBuf()
{
    if (NULL != mFd)
    {
        fclose(mFd);
    }
}

size_t FileBuf::Write( void *buf, size_t len )
{
    if (NULL == mFd)
    {
        return 0;
    }
    return fwrite(buf, 1, len, mFd);
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

bool FileBuf::Flush()
{
    if (NULL == mFd)
    {
        return false;
    }
    return fflush(mFd) == 0;
}

}
