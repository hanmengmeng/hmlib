#ifndef HM_FILE_H
#define HM_FILE_H

#include "hm_common.h"
#include "hm_hash.h"

namespace hm
{

class IFileBuf
{
public:
    ~IFileBuf(){}
    virtual bool Open(const wchar_t *path, int mode) = 0;
    virtual void Close() = 0;
    virtual size_t Write(void *buf, size_t len) = 0;
    virtual size_t Read(void *buf, size_t len) = 0;
    virtual hm_long_64 GetFileSize() = 0;
    virtual bool Flush(void *buf, size_t len) = 0;
    virtual bool Reserve(void **buf, size_t len) = 0;
};

class IHash
{
public:
    ~IHash(){}
    virtual void HashReset() = 0;
    virtual bool HashResult(unsigned char *hashResult, size_t len) = 0;
};

// Read or write file in binary mode,
// and have write buffer function
class FileBuf : public IFileBuf
{
public:
    enum
    {
        FILE_MODE_READ = 1 << 0,
        FILE_MODE_WRITE = 1 << 1,
        FILE_MODE_APPEND = 1 << 2,
    };
    FileBuf();
    FileBuf(const wchar_t *path, int mode);
    FileBuf(const wchar_t *path, int mode, size_t bufSize);
    ~FileBuf();
    virtual bool Open(const wchar_t *path, int mode);
    virtual void Close();
    virtual size_t Write(void *buf, size_t len);
    virtual size_t Read(void *buf, size_t len);
    virtual hm_long_64 GetFileSize();
    virtual bool Flush(void *buf, size_t len);
    virtual bool Reserve(void **buf, size_t len);

private:
    FileBuf(const FileBuf &fp);
    FileBuf &operator=(const FileBuf &fp);
    void Init(const wchar_t *path, int mode);

protected:
    FILE *mFd;
    hm_long_64 mFileSize;
    char *mBuf;
    size_t mCurrPos;
    size_t mBufSize;
};

class HashFileBuf : public FileBuf, public IHash
{
public:
    HashFileBuf();
    HashFileBuf(const wchar_t *path, int mode);
    HashFileBuf(const wchar_t *path, int mode, size_t bufSize);
    ~HashFileBuf();
    virtual void HashReset();
    virtual bool HashResult(unsigned char *hashResult, size_t len);
    virtual bool Flush(void *buf, size_t len);
    virtual size_t Read(void *buf, size_t len);

private:
    Sha1Hash mHash;
};

} // namespace hm
#endif
