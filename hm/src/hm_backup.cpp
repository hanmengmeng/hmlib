#include "hm_backup.h"

#include <stdio.h>

#include "hm_file_buf.h"
#include "hm_string.h"
#include "hm_dir.h"

#define entry_disk_size(type,len) ((offsetof(type, path) + (len) + 8) & ~7)
#define entry_size_except_path(type) (offsetof(type, path))

#define INDEX_HEADER_LEN 64
#define INDEX_VERSION_STRING "version:1.0"

#define FILE_PATH_MAX 4096

#define READ_FILE_BUFFER_SIZE 4096

namespace hm
{

class FileIndex
{
public:
    FileIndex(const wchar_t *indexFilePath);
    ~FileIndex();

    bool AddFile(const wchar_t *filePath);
    bool AddEntry(const FileEntry &entry);
    bool Save();
    bool Load();
    int Find(const wchar_t *filePath);
    bool Remove(const wchar_t *filePath);

private:
    bool WriteFileEntry(IFileBuf *fileBuffer, FileEntry *entry);
    bool WriteEntries(IFileBuf *fileBuffer);
    int Find(const char *filePath);
    bool Remove(const char *filePath);
    size_t ReadFileEntry(FileEntry *entry, const char *buffer, size_t len);
    bool ReadEntries(const char *buffer, size_t len, size_t entriesCount);

private:
    wchar_t *mIndexFilePath;
    time_t mLastModified;
    std::vector<FileEntry> mEntries;
};

class DirectBlob : public IBlob
{
public:
    DirectBlob(const wchar_t *backupDir);
    ~DirectBlob();

    virtual bool CreateBlob(object_id &outObjId, const wchar_t *filePath);
private:
    hm_wstring mBackupDir;
};

DirectBlob::DirectBlob( const wchar_t *backupDir )
{
    mBackupDir = backupDir;
}

DirectBlob::~DirectBlob()
{

}

bool DirectBlob::CreateBlob( object_id &outObjId, const wchar_t *filePath )
{
    hm_wstring tmpFile = mBackupDir;
    DirUtil::MakeTempFile(tmpFile);

    HashFileBuf hrfb(filePath, FileBuf::FILE_MODE_READ);
    FileBuf wfb(tmpFile.c_str(), FileBuf::FILE_MODE_WRITE);

    unsigned char buffer[READ_FILE_BUFFER_SIZE];
    size_t readSize = 0;
    while ((readSize = hrfb.Read(buffer, READ_FILE_BUFFER_SIZE)) > 0)
    {
        if (wfb.Write(buffer, readSize) != readSize)
        {

        }
    }
    return true;
}

FileIndex::FileIndex( const wchar_t *indexFilePath )
{
    mIndexFilePath = _wcsdup(indexFilePath);
    mLastModified = 0;
}

FileIndex::~FileIndex()
{
    free(mIndexFilePath);
    for (size_t i = 0; i < mEntries.size(); i++)
    {
        free(mEntries.at(i).path);
    }
}

bool FileIndex::Save()
{
    HashFileBuf hashFb(mIndexFilePath, FileBuf::FILE_MODE_WRITE, 4096);

    // Write the index header
    char *headerBuffer;
    hashFb.Reserve((void**)&headerBuffer, INDEX_HEADER_LEN);
    memset(headerBuffer, 0, INDEX_HEADER_LEN);
    strcpy(headerBuffer, INDEX_VERSION_STRING);
    sprintf(headerBuffer+strlen(INDEX_VERSION_STRING)+1, "%u", mEntries.size());

    // Write all entries
    if (!WriteEntries(&hashFb))
    {
        return false;
    }

    hashFb.Flush(NULL, 0);

    // Write hash value at end
    unsigned char hashResult[HASH_SHA1_LEN];
    hashFb.HashResult(hashResult, HASH_SHA1_LEN);
    hashFb.Write(hashResult, HASH_SHA1_LEN);
    return true;
}

bool FileIndex::Load()
{
    // Check index file exist
    if (_taccess(mIndexFilePath, 0) != 0)
    {
        return false;
    }
    _stat64 st;
    if (0 != _tstat64(mIndexFilePath, &st))
    {
        return false;
    }
    if (st.st_size < HASH_SHA1_LEN + INDEX_HEADER_LEN)
    {
        return false;
    }

    size_t entriesSize = st.st_size - HASH_SHA1_LEN;
    char *indexBuffer = new char[entriesSize];
    HashFileBuf fb(mIndexFilePath, FileBuf::FILE_MODE_READ);
    size_t readSize = fb.Read(indexBuffer, entriesSize);
    if (readSize != entriesSize)
    {
        delete []indexBuffer;
        return false;
    }

    unsigned char sha1[HASH_SHA1_LEN];
    fb.HashResult(sha1, HASH_SHA1_LEN);

    unsigned char sha1Disk[HASH_SHA1_LEN];
    fb.Read(sha1Disk, HASH_SHA1_LEN);
    if (memcmp(sha1, sha1Disk, HASH_SHA1_LEN) != 0)
    {
        return false; // Sha1 hash verify failed
    }

    std::string ver = indexBuffer;
    char *countStr = indexBuffer+ver.length()+1;
    size_t count = strtoul(countStr, 0, 10);

    return ReadEntries(indexBuffer, st.st_size - INDEX_HEADER_LEN, count);
}

bool FileIndex::WriteFileEntry( IFileBuf *fileBuffer, FileEntry *entry )
{
    if (NULL == fileBuffer)
    {
        return false;
    }
    size_t pathLen = strlen(entry->path);
    size_t diskLen = entry_disk_size(FileEntry, pathLen);
    void *mem = NULL;

    if (!fileBuffer->Reserve(&mem, diskLen))
    {
        return false;
    }
    memset(mem, 0, diskLen);
    size_t memLen = entry_size_except_path(FileEntry);
    memcpy(mem, entry, memLen);
    memcpy((char*)mem+memLen, entry->path, pathLen);
    return true;
}

bool FileIndex::WriteEntries(IFileBuf *fileBuffer)
{
    if (NULL == fileBuffer)
    {
        return false;
    }
    for (size_t i = 0; i < mEntries.size(); i++)
    {
        if (!WriteFileEntry(fileBuffer, &mEntries.at(i)))
        {
            return false;
        }
    }
    return true;
}

bool FileIndex::AddEntry( const FileEntry &entry )
{
    // If entry exist, remove it first
    Remove(entry.path);
    mEntries.push_back(entry);
    return true;
}

int FileIndex::Find( const wchar_t *filePath )
{
    std::string findStr = StringConvert(filePath).ToUtf8();
    return Find(findStr.c_str());
}

int FileIndex::Find( const char *filePath )
{
    for (size_t i = 0; i < mEntries.size(); i++)
    {
        if (strcmp(mEntries.at(i).path, filePath) == 0)
        {
            return i;
        }
    }
    return -1;
}

bool FileIndex::Remove( const wchar_t *filePath )
{
    std::string findStr = StringConvert(filePath).ToUtf8();
    return Remove(findStr.c_str());
}

bool FileIndex::Remove( const char *filePath )
{
    int pos = 0;
    if ((pos = Find(filePath)) >= 0)
    {
        mEntries.erase(mEntries.begin() + pos);
    }
    return true;
}

size_t FileIndex::ReadFileEntry( FileEntry *entry, const char *buffer, size_t len )
{
    if (NULL == entry || NULL == buffer)
    {
        return 0;
    }

    // First, read whole FileEntry except path element
    size_t entrySize = entry_size_except_path(FileEntry);
    if (entrySize + HASH_SHA1_LEN >= len)
    {
        return 0;
    }

    memcpy(entry, buffer, entrySize);
    const char *path = buffer+entrySize;
    size_t pathLen = strlen(path);
    // Ensure file path not exceed maximum limit,
    // max file path is 4096 on Linux
    if (pathLen > FILE_PATH_MAX)
    {
        return 0;
    }
    entry->path = _strdup(path);

    // return current entry size that occupy disk's space
    return entry_disk_size(FileEntry, strlen(path));
}

bool FileIndex::ReadEntries( const char *buffer, size_t len, size_t entriesCount )
{
    if (NULL == buffer || 0 == len)
    {
        return false;
    }

    FileEntry entry;
    size_t beginPos = INDEX_HEADER_LEN;
    size_t leftIndexBuffer = len;
    size_t entrySize = 0;
    for (size_t i = 0; i < entriesCount; i++)
    {
        entrySize = ReadFileEntry(&entry, buffer+beginPos, leftIndexBuffer);
        if (entrySize <= 0)
        {
            return false;
        }
        beginPos += entrySize;
        leftIndexBuffer -= entrySize;
        mEntries.push_back(entry);
    }
    return true;
}

BackupMgr::BackupMgr(const wchar_t *backupDir)
{
    hm_wstring backupDirPath = DirUtil::MakePathRegular(backupDir);
    hm_wstring backupIndexFilePath = backupDirPath;
    if (backupIndexFilePath.at(backupIndexFilePath.size()-1) != _T('\\'))
    {
        backupIndexFilePath.push_back(_T('\\'));
    }
    backupIndexFilePath.append(_T("index"));
    DirUtil::CreateParentDirectory(backupIndexFilePath);

    mFi = new FileIndex(backupIndexFilePath.c_str());
    mFi->Load();
}

BackupMgr::~BackupMgr()
{
    delete mFi;
}

int BackupMgr::AddDir( const wchar_t *filePath, const wchar_t *relPath )
{
    return 0;
}

int BackupMgr::AddFile( const wchar_t *filePath, const wchar_t *relPath )
{
    FileEntry en;
    memset(&en, 0, sizeof(FileEntry));
    en.file_size = 10;
    en.path = "D:/test_git_workspace/test.xml";
    mFi->AddEntry(en);
    return 0;
}

int BackupMgr::RemoveFile( const wchar_t *relPath )
{
    return 0;
}

int BackupMgr::Finish( object_id &oid )
{
    mFi->Save();
    return 0;
}

int BackupMgr::GetFileList( const object_id &indexOid, std::vector<FileEntry> &fileList )
{
    return 0;
}

}
