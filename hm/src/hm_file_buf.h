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
    virtual bool Open(const t_char *path, int mode) = 0;
    virtual void Close() = 0;
    virtual t_size Write(void *buf, t_size len) = 0;
    virtual t_size Read(void *buf, t_size len) = 0;
    virtual t_size GetFileSize() = 0;
    virtual bool Flush(void *buf, t_size len) = 0;
    virtual bool Reserve(void **buf, t_size len) = 0;
    virtual const t_char *GetFilePath() = 0;
};

class IHash
{
public:
    ~IHash(){}
    virtual void HashReset() = 0;
    virtual bool HashResult(unsigned char *hashResult, t_size len) = 0;
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
    FileBuf(const t_char *path, int mode);
    FileBuf(const t_char *path, int mode, t_size bufSize);
    ~FileBuf();
    virtual bool Open(const t_char *path, int mode);
    virtual void Close();
    virtual t_size Write(void *buf, t_size len);
    virtual t_size Read(void *buf, t_size len);
    virtual t_size GetFileSize();
    virtual bool Flush(void *buf, t_size len);
    virtual bool Reserve(void **buf, t_size len);
    virtual const t_char *GetFilePath();

private:
    FileBuf(const FileBuf &fp);
    FileBuf &operator=(const FileBuf &fp);
    void Init(const t_char *path, int mode);

protected:
    FILE *mFd;
    t_size mFileSize;
    char *mBuf;
    t_size mCurrPos;
    t_size mBufSize;
    t_string mFilePath;
};

class HashFileBuf : public FileBuf, public IHash
{
public:
    HashFileBuf();
    HashFileBuf(const t_char *path, int mode);
    HashFileBuf(const t_char *path, int mode, t_size bufSize);
    ~HashFileBuf();
    virtual void HashReset();
    virtual bool HashResult(unsigned char *hashResult, t_size len);
    virtual bool Flush(void *buf, t_size len);
    virtual t_size Read(void *buf, t_size len);

private:
    Sha1Hash mHash;
};

} // namespace hm
#endif
