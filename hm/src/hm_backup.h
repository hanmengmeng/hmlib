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

typedef std::vector<FileEntry> FileEntryList;

class IBackup
{
public:
    virtual ~IBackup(){}

    virtual bool AddDir(const t_char *filePath, const t_char *relPath) = 0;
    virtual bool AddDir(const t_char *filePath, const t_char *relPath, const struct _stat64 *st) = 0;
    virtual bool AddFile(const t_char *filePath, const t_char *relPath) = 0;
    virtual bool AddFile(const t_char *filePath, const t_char *relPath, const struct _stat64 *st) = 0;
    virtual bool RemoveFile(const t_char *relPath) = 0;
    virtual bool Finish(const char *tagName) = 0;
    virtual int GetFileList(FileEntryList &fileList) = 0;
    virtual t_error GetLastError() = 0;
    virtual bool GetTagList(std::vector<t_string> &tags) = 0;
    virtual bool RetrieveFile(const object_id &oid, const t_char *destPath) = 0;
    virtual bool RetrieveFile(const t_char *relPath, const t_char *destPath) = 0;
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
    virtual bool Finish(const char *tagName);
    virtual int GetFileList(FileEntryList &fileList);
    virtual t_error GetLastError();
    virtual bool GetTagList(std::vector<t_string> &tags);
    virtual bool RetrieveFile(const object_id &oid, const t_char *destPath);
    virtual bool RetrieveFile(const t_char *relPath, const t_char *destPath);
    bool SetTag(const t_char *tagName);

private:
    void InitFileEntry(FileEntry *fe, const struct _stat64 *st);
    t_string GetTagPath();
    bool GetTagOid(const t_string &tagName, object_id &oid);
    bool GetTagOid(const t_string &tagName, std::string &oid);

private:
    FileIndex *mFi;
    t_error mLastError;
    t_string mBackupDir;
};

} // namespace hm
#endif
