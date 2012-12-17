#ifndef HM_FILE_H
#define HM_FILE_H

#include "hm_common.h"

namespace hm
{

class IFileBuf
{
public:
    ~IFileBuf(){}
    virtual size_t Write(void *buf, size_t len) = 0;
    virtual size_t Read(void *buf, size_t len) = 0;
    virtual hm_long_64 GetFileSize() = 0;
    virtual bool Flush() = 0;
};

// Read or write file in binary mode
class FileBuf : public IFileBuf
{
public:
    enum
    {
        FILE_MODE_READ = 1 << 0,
        FILE_MODE_WRITE = 1 << 1,
        FILE_MODE_APPEND = 1 << 2,
    };
    FileBuf(const hm_string &path, int mode);
    ~FileBuf();
    virtual size_t Write(void *buf, size_t len);
    virtual size_t Read(void *buf, size_t len);
    virtual hm_long_64 GetFileSize();
    virtual bool Flush();

private:
    FILE *mFd;
    hm_long_64 mFileSize;
};

} // namespace hm
#endif
