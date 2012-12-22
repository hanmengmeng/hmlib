#ifndef HM_BACKUP_H
#define HM_BACKUP_H

#include <vector>
#include "hm_common.h"

namespace hm
{

typedef struct _FileEntry {
    time_t ctime;
    time_t mtime;

    unsigned int dev;
    unsigned int ino;
    unsigned int mode;
    unsigned int uid;
    unsigned int gid;
    t_long_64 file_size;

    object_id oid;

    unsigned short flags;

    char *path;
} FileEntry;

class IBackup
{
public:
    virtual ~IBackup(){}

    virtual bool AddDir(const t_char *filePath, const t_char *relPath) = 0;
    virtual bool AddDir(const t_char *filePath, const t_char *relPath, const struct _stat64 *st) = 0;
    virtual bool AddFile(const t_char *filePath, const t_char *relPath) = 0;
    virtual bool AddFile(const t_char *filePath, const t_char *relPath, const struct _stat64 *st) = 0;
    virtual bool RemoveFile(const t_char *relPath) = 0;
    virtual bool Finish(object_id &oid) = 0;
    virtual int GetFileList(const object_id &indexOid, std::vector<FileEntry> &fileList) = 0;
    virtual t_error GetLastError() = 0;
};

class IBlob
{
public:
    virtual ~IBlob(){}
    virtual bool CreateBlob(object_id &outObjId, const t_char *filePath) = 0;
    //virtual void SetBlobDirectory(const t_char *blobDir) = 0;

    //virtual bool Init(t_size fileSize) = 0;
    virtual t_size Write(void *buf, t_size len) = 0;
    //virtual bool Finish(object_id &outOid) = 0;
    virtual bool WriteFinish(object_id &outOid) = 0;

    //virtual bool Init(const object_id &oid) = 0;
    virtual t_size Read(void *buf, t_size len) = 0;
    //virtual bool Finish() = 0;

    virtual t_size GetBlobSize() = 0;
};

class BlobObjectFactory
{
public:
    static IBlob* GetDirectBlob();
};

class FileIndex;
class BackupMgr : public IBackup
{
public:
    BackupMgr(const t_char *backupDir);
    ~BackupMgr();
    virtual bool AddDir(const t_char *filePath, const t_char *relPath);
    virtual bool AddDir(const t_char *filePath, const t_char *relPath, const struct _stat64 *st);
    virtual bool AddFile(const t_char *filePath, const t_char *relPath);
    virtual bool AddFile(const t_char *filePath, const t_char *relPath, const struct _stat64 *st);
    virtual bool RemoveFile(const t_char *relPath);
    virtual bool Finish(object_id &oid);
    virtual int GetFileList(const object_id &indexOid, std::vector<FileEntry> &fileList);
    virtual t_error GetLastError();
    virtual void GetTagList(std::vector<std::string> &tags);

    void InitFileEntry(FileEntry *fe, const struct _stat64 *st);

private:
    FileIndex *mFi;
    //IBlob *mBlober;
    t_error mLastError;
    t_string mBackupDir;
};

} // namespace hm
#endif
