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
    size_t file_size;

    object_id oid;

    unsigned short flags;
    unsigned short flags_extended;

    char *path;
} FileEntry;

class IBackup
{
public:
    virtual ~IBackup(){}

    virtual int AddDir(const wchar_t *filePath, const wchar_t *relPath) = 0;
    virtual int AddFile(const wchar_t *filePath, const wchar_t *relPath) = 0;
    virtual int RemoveFile(const wchar_t *relPath) = 0;
    virtual int Finish(object_id &oid) = 0;
    virtual int GetFileList(const object_id &indexOid, std::vector<FileEntry> &fileList) = 0;

};

class IBlob
{
public:
    virtual ~IBlob(){}
    virtual bool CreateBlob(object_id &outObjId, const wchar_t *filePath) = 0;
};

class FileIndex;
class BackupMgr : public IBackup
{
public:
    BackupMgr(const wchar_t *backupDir);
    ~BackupMgr();
    virtual int AddDir(const wchar_t *filePath, const wchar_t *relPath);
    virtual int AddFile(const wchar_t *filePath, const wchar_t *relPath);
    virtual int RemoveFile(const wchar_t *relPath);
    virtual int Finish(object_id &oid);
    virtual int GetFileList(const object_id &indexOid, std::vector<FileEntry> &fileList);

private:
    FileIndex *mFi;
};

} // namespace hm
#endif
