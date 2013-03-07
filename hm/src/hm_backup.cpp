#include "hm_backup.h"

#include <stdio.h>
#include <assert.h>

#include "hm_file_buf.h"
#include "hm_string.h"
#include "hm_dir.h"

#define entry_disk_size(type,len) ((offsetof(type, path) + (len) + 8) & ~7)
#define entry_size_except_path(type) (offsetof(type, path))

#define INDEX_HEADER_LEN 64
#define INDEX_VERSION_STRING "version:1.0"

#define FILE_PATH_MAX 4096

#define READ_FILE_BUFFER_SIZE 4096

#define BACKUP_MAX_FILE_SIZE 4294967295 // 4G

#define BLOB_FILE_HEADER_LEN 32

namespace hm
{

class FileIndex
{
public:
    FileIndex(const t_char *indexFilePath);
    FileIndex(void *buffer, t_size len);
    ~FileIndex();

    bool AddFile(const t_char *filePath);
    bool AddEntry(const FileEntry &entry);
    bool Save(const t_char *indexPath);
    int Find(const t_char *filePath);
    bool Remove(const t_char *filePath);
    t_size GetEntryCount();
    FileEntry *Get(t_size pos);
    t_char *GetIndexPath();

private:
    bool WriteFileEntry(IFileBuf *fileBuffer, FileEntry *entry);
    bool WriteEntries(IFileBuf *fileBuffer);
    int Find(const char *filePath);
    bool Remove(const char *filePath);
    t_size ReadFileEntry(FileEntry *entry, const char *buffer, t_size len);
    bool ReadEntries(const char *buffer, t_size len, t_size entriesCount);
    bool Load(const t_char *indexPath);
    bool ParseIndex(void *buffer, t_size len);

private:
    t_char *mIndexFilePath;
    time_t mLastModified;
    FileEntryList mEntries;
};

class DirectBlob : public IBlob
{
public:
    // Write blob
    DirectBlob(const t_char *blobDir, t_size fileSize);
    // Read blob
    DirectBlob(const t_char *blobDir, const object_id &oid);
    ~DirectBlob();

    // Implement interface IBlob
    virtual bool CreateBlob(object_id &outObjId, const t_char *filePath);
    virtual t_size Write(void *buf, t_size len);
    virtual bool WriteFinish(object_id &outOid);
    virtual t_size Read(void *buf, t_size len);
    virtual t_size GetBlobSize();

private:
    bool WriteBlobHeader(IFileBuf *fb, t_size filesize);
    bool WriteBlob(IFileBuf *fb, void *buf, t_size len);
    bool CreateBlobFile(object_id &outObjId, HashFileBuf *fb);
    size_t ReadBlobHeader(IFileBuf *fb, void *buf, t_size len);
    t_size ReadBlob(IFileBuf *fb, void *buf, t_size len);
    t_string GetBlobPath(const object_id &oid);

private:
    t_string mBackupDir;
    HashFileBuf *mFileBuf;
};

typedef struct
{
    unsigned int removed;
    unsigned int attr;
    object_id oid;
    size_t filename_len;
    char filename[1];
}TreeEntry;

typedef std::vector<TreeEntry> TreeEntryList;
class FileTree
{
public:

};

DirectBlob::DirectBlob( const t_char *blobDir, t_size fileSize )
{
    mBackupDir = blobDir;
    t_string tmpFile = blobDir;
    DirUtil::MakeTempFile(tmpFile);
    mFileBuf = new HashFileBuf (tmpFile.c_str(), FileBuf::FILE_MODE_WRITE);

    if (!WriteBlobHeader(mFileBuf, fileSize))
    {
        delete mFileBuf;
        mFileBuf = NULL;
    }
}

DirectBlob::DirectBlob( const t_char *blobDir, const object_id &oid )
{
    mBackupDir = blobDir;
    t_string blobFilePath = GetBlobPath(oid);
    mFileBuf = new HashFileBuf (blobFilePath.c_str(), FileBuf::FILE_MODE_READ);
    char buffer[BLOB_FILE_HEADER_LEN];

    if (ReadBlobHeader(mFileBuf, buffer, BLOB_FILE_HEADER_LEN) != BLOB_FILE_HEADER_LEN)
    {
        delete mFileBuf;
        mFileBuf = NULL;
    }
}

DirectBlob::~DirectBlob()
{
    if (NULL != mFileBuf)
    {
        delete mFileBuf;
        mFileBuf = NULL;
    }
}

bool DirectBlob::CreateBlob( object_id &outObjId, const t_char *filePath )
{
    if (NULL == mFileBuf)
    {
        return false;
    }

    FileBuf readBuf(filePath, FileBuf::FILE_MODE_READ);
    unsigned char buffer[READ_FILE_BUFFER_SIZE];
    t_size readSize = 0;
    while ((readSize = readBuf.Read(buffer, READ_FILE_BUFFER_SIZE)) > 0)
    {
        if (!WriteBlob(mFileBuf, buffer, readSize))
        {
            return false;
        }
    }
    return CreateBlobFile(outObjId, mFileBuf);
}

bool DirectBlob::WriteBlob( IFileBuf *fb, void *buf, t_size len )
{
    if (fb->Write(buf, len) != len)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool DirectBlob::WriteBlobHeader( IFileBuf *fb, t_size filesize )
{
    char headerBuf[BLOB_FILE_HEADER_LEN];
    t_size headerLen;
    headerLen = _snprintf_s(headerBuf, BLOB_FILE_HEADER_LEN, "%s %d", "blob", filesize);
    headerLen++; // include NULL terminator

    if (fb->Write(headerBuf, BLOB_FILE_HEADER_LEN) != BLOB_FILE_HEADER_LEN)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool DirectBlob::CreateBlobFile( object_id &outObjId, HashFileBuf *fb )
{
    t_string tmpFilePath = fb->GetFilePath();
    unsigned char sha1[HASH_SHA1_LEN];
    fb->HashResult(sha1, HASH_SHA1_LEN);
    memcpy(&outObjId, sha1, HASH_SHA1_LEN);
    fb->Close();

    t_string strObjId = StringConvert(Sha1Hash::OidToString(outObjId).c_str()).ToUtf16();
    t_string targetPath = DirUtil::MakeFilePath(mBackupDir, strObjId);

    if (!DirUtil::IsFileExist(targetPath))
    {
        return DirUtil::MoveFile(tmpFilePath, targetPath);
    }
    else
    {
        DirUtil::DeleteFileIfExist(tmpFilePath);
        return true; // Blob file already exist
    }
}

t_size DirectBlob::Write( void *buf, t_size len )
{
    if (NULL == mFileBuf)
    {
        return 0;
    }
    if (!WriteBlob(mFileBuf, buf, len))
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool DirectBlob::WriteFinish( object_id &outOid )
{
    bool ret = true;
    if (NULL != mFileBuf)
    {
        ret = CreateBlobFile(outOid, mFileBuf);
        delete mFileBuf;
        mFileBuf = NULL;
    }
    return ret;
}

t_size DirectBlob::Read( void *buf, t_size len )
{
    return ReadBlob(mFileBuf, buf, len);
}

t_string DirectBlob::GetBlobPath( const object_id &oid )
{
    t_string oidName = StringConvert(Sha1Hash::OidToString(oid)).ToUtf16();
    return DirUtil::MakeFilePath(mBackupDir, oidName);
}

t_size DirectBlob::ReadBlobHeader( IFileBuf *fb, void *buf, t_size len )
{
    if (NULL == fb || NULL == buf)
    {
        return 0;
    }
    if (len >= BLOB_FILE_HEADER_LEN)
    {
        t_size readLen = 0;
        if ((readLen = fb->Read(buf, BLOB_FILE_HEADER_LEN)) > 0)
        {
            return readLen;
        }
    }
    return 0;
}

t_size DirectBlob::ReadBlob( IFileBuf *fb, void *buf, t_size len )
{
    if (NULL == fb || NULL == buf)
    {
        return 0;
    }
    t_size readLen = 0;
    if ((readLen = fb->Read(buf, len)) > 0)
    {
        return readLen;
    }
    return 0;
}

t_size DirectBlob::GetBlobSize()
{
    if (NULL == mFileBuf)
    {
        return 0;
    }
    return mFileBuf->GetFileSize() - BLOB_FILE_HEADER_LEN;
}

FileIndex::FileIndex( const t_char *indexFilePath )
{
    mIndexFilePath = _wcsdup(indexFilePath);
    mLastModified = 0;
    Load(mIndexFilePath);
}

FileIndex::FileIndex( void *buffer, t_size len )
{
    mIndexFilePath = NULL;
    mLastModified = 0;
    ParseIndex(buffer, len);
}

FileIndex::~FileIndex()
{
    if (NULL != mIndexFilePath)
    {
        free(mIndexFilePath);
    }

    for (t_size i = 0; i < mEntries.size(); i++)
    {
        free(mEntries.at(i).path);
    }
}

bool FileIndex::Save(const t_char *indexPath)
{
    if (NULL == indexPath)
    {
        return false;
    }
    HashFileBuf hashFb(indexPath, FileBuf::FILE_MODE_WRITE, 4096);

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

bool FileIndex::Load(const t_char *indexPath)
{
    // Check index file exist
    if (_taccess(indexPath, 0) != 0)
    {
        return false;
    }
    _stat64 st;
    if (0 != _tstat64(indexPath, &st))
    {
        return false;
    }
    if (st.st_size < HASH_SHA1_LEN + INDEX_HEADER_LEN)
    {
        return false;
    }

    t_size indexSize = st.st_size;
    char *indexBuffer = new char[indexSize];
    FileBuf fb(indexPath, FileBuf::FILE_MODE_READ);
    t_size readSize = fb.Read(indexBuffer, indexSize);
    if (readSize != indexSize)
    {
        delete []indexBuffer;
        return false;
    }

    return ParseIndex(indexBuffer, indexSize);
}

bool FileIndex::WriteFileEntry( IFileBuf *fileBuffer, FileEntry *entry )
{
    if (NULL == fileBuffer)
    {
        return false;
    }
    t_size pathLen = strlen(entry->path);
    t_size diskLen = entry_disk_size(FileEntry, pathLen);
    void *mem = NULL;

    if (!fileBuffer->Reserve(&mem, diskLen))
    {
        return false;
    }
    memset(mem, 0, diskLen);
    t_size memLen = entry_size_except_path(FileEntry);
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
    for (t_size i = 0; i < mEntries.size(); i++)
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

int FileIndex::Find( const t_char *filePath )
{
    std::string findStr = StringConvert(filePath).ToUtf8();
    return Find(findStr.c_str());
}

int FileIndex::Find( const char *filePath )
{
    for (t_size i = 0; i < mEntries.size(); i++)
    {
        if (strcmp(mEntries.at(i).path, filePath) == 0)
        {
            return i;
        }
    }
    return -1;
}

bool FileIndex::Remove( const t_char *filePath )
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

t_size FileIndex::ReadFileEntry( FileEntry *entry, const char *buffer, t_size len )
{
    if (NULL == entry || NULL == buffer)
    {
        return 0;
    }

    // First, read whole FileEntry except path element
    t_size entrySize = entry_size_except_path(FileEntry);
    if (entrySize + HASH_SHA1_LEN >= len)
    {
        return 0;
    }

    memcpy(entry, buffer, entrySize);
    const char *path = buffer+entrySize;
    t_size pathLen = strlen(path);
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

bool FileIndex::ReadEntries( const char *buffer, t_size len, t_size entriesCount )
{
    if (NULL == buffer || 0 == len)
    {
        return false;
    }

    FileEntry entry;
    t_size beginPos = INDEX_HEADER_LEN;
    t_size leftIndexBuffer = len;
    t_size entrySize = 0;
    for (t_size i = 0; i < entriesCount; i++)
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

t_size FileIndex::GetEntryCount()
{
    return mEntries.size();
}

FileEntry *FileIndex::Get( t_size pos )
{
    if (mEntries.size() > 0 && pos < mEntries.size())
    {
        return &mEntries.at(pos);
    }
    else
    {
        return NULL;
    }
}

t_char * FileIndex::GetIndexPath()
{
    return mIndexFilePath;
}

bool FileIndex::ParseIndex( void *buffer, t_size len )
{
    Sha1Hash sh;
    // Compute entries SHA1 hash
    sh.Update(buffer, len - HASH_SHA1_LEN);
    unsigned char sha1[HASH_SHA1_LEN];
    sh.Final(sha1);

    // Compare the SHA1 hash with the hash on disk
    if (memcmp(sha1, (char*)buffer + len - HASH_SHA1_LEN, HASH_SHA1_LEN) != 0)
    {
        return false; // Sha1 hash verify failed
    }

    // Parse index header
    // version:1.0 [count]
    std::string ver = (char*)buffer;
    char *countStr = (char*)buffer+ver.length()+1;
    t_size count = strtoul(countStr, 0, 10);

    return ReadEntries((char*)buffer, len - INDEX_HEADER_LEN, count);
}

BackupMgr::BackupMgr(const t_char *backupDir)
{
    mLastError = 0;
    t_string backupDirPath = DirUtil::MakePathRegular(backupDir);
    t_string backupIndexFilePath = backupDirPath;
    mBackupDir = backupIndexFilePath;

    if (backupIndexFilePath.at(backupIndexFilePath.size()-1) != _T('\\'))
    {
        backupIndexFilePath.push_back(_T('\\'));
    }
    backupIndexFilePath.append(_T("index"));
    DirUtil::CreateParentDirectory(backupIndexFilePath);

    mFi = new FileIndex(backupIndexFilePath.c_str());
}

BackupMgr::~BackupMgr()
{
    if (NULL != mFi)
    {
        delete mFi;
    }
}

bool BackupMgr::AddDir( const t_char *filePath, const t_char *relPath )
{
    _stat64 st;
    if (0 != _tstat64(filePath, &st))
    {
        return false;
    }
    return AddDir(filePath, relPath, &st);
}

bool BackupMgr::AddDir( const t_char *filePath, const t_char *relPath, const struct _stat64 *st )
{
    object_id oid;
    DirectBlob db(mBackupDir.c_str(), 0);
    if (!db.Write("", 0))
    {
        return false;
    }
    if (!db.WriteFinish(oid))
    {
        return false;
    }

    FileEntry en;
    memset(&en, 0, sizeof(FileEntry));

    InitFileEntry(&en, st);
    en.oid = oid;

    std::string p = StringConvert(relPath).ToUtf8();
    en.path = _strdup(p.c_str());
    mFi->AddEntry(en);
    return true;
}

bool BackupMgr::AddFile( const t_char *filePath, const t_char *relPath )
{
    _stat64 st;
    if (0 != _tstat64(filePath, &st))
    {
        return false;
    }
    return AddFile(filePath, relPath, &st);
}

bool BackupMgr::AddFile( const t_char *filePath, const t_char *relPath, const struct _stat64 *st )
{
    object_id oid;
    DirectBlob db(mBackupDir.c_str(), st->st_size);
    if (!db.CreateBlob(oid, filePath))
    {
        return false;
    }

    FileEntry en;
    memset(&en, 0, sizeof(FileEntry));

    InitFileEntry(&en, st);
    en.oid = oid;

    std::string p = StringConvert(relPath).ToUtf8();
    en.path = _strdup(p.c_str());
    mFi->AddEntry(en);
    return true;
}

bool BackupMgr::RemoveFile( const t_char *relPath )
{
    return mFi->Remove(relPath);
}

bool BackupMgr::Finish( const char *tagName )
{
    if (!mFi->Save(mFi->GetIndexPath()))
    {
        return false;
    }
    _stat64 st;
    if (0 != _tstat64(mFi->GetIndexPath(), &st))
    {
        return false;
    }

    DirectBlob db(mBackupDir.c_str(), st.st_size);
    object_id oid;
    if (!db.CreateBlob(oid, mFi->GetIndexPath()))
    {
        return false;
    }

    // Create tag file at [BackupDir]/tags/ directory
    t_string tagPath = GetTagPath();
    //t_string strOid = StringConvert(Sha1Hash::OidToString(oid).c_str()).ToUnicode();
    t_string strTag = StringConvert(tagName).ToUtf16();
    t_string tagFilePath = DirUtil::MakeFilePath(tagPath, strTag);
    if (!DirUtil::CreateParentDirectory(tagFilePath))
    {
        return false;
    }
    FileBuf fb(tagFilePath.c_str(), FileBuf::FILE_MODE_WRITE);
    std::string strOid = Sha1Hash::OidToString(oid);
    fb.Write((void*)strOid.c_str(),strOid.size());
    return true;
}

int BackupMgr::GetFileList( FileEntryList &fileList )
{
    fileList.clear();
    for (size_t i = 0; i < mFi->GetEntryCount(); i++)
    {
        fileList.push_back(*mFi->Get(i));
    }
    return fileList.size();
}

t_error BackupMgr::GetLastError()
{
    return mLastError;
}

void BackupMgr::InitFileEntry( FileEntry *fe, const struct _stat64 *st )
{
    fe->ctime = st->st_ctime;
    fe->dev = st->st_dev;
    fe->file_size = st->st_size;
    fe->flags = 0;
    fe->gid = st->st_gid;
    fe->ino = st->st_ino;
    fe->mode = st->st_mode;
    fe->mtime = st->st_mtime;
    fe->uid = st->st_uid;
}

bool BackupMgr::GetTagList( std::vector<t_string> &tags )
{
    t_string tagPath = GetTagPath();
    //std::vector<t_string> fileList;
    if (DirUtil::EnumFileNames(tagPath, tags))
    {
#if 0
        for (size_t i = 0; i < fileList.size(); i++)
        {
            std::string oid;
            if (!GetTagOid(fileList.at(i), oid))
            {
                continue;
            }
            tags.push_back(StringConvert(oid).ToUtf16());
        }
#endif
    }
    return tags.size() > 0;
}

bool BackupMgr::RetrieveFile( const object_id &oid, const t_char *destPath )
{
    DirectBlob db(mBackupDir.c_str(), oid);
    FileBuf fb(destPath, FileBuf::FILE_MODE_WRITE);
    const size_t BufSize = 4096;
    char buf[BufSize];
    size_t readlen = 0;
    while ((readlen = db.Read(buf, BufSize)) > 0)
    {
        if (readlen != fb.Write(buf, readlen))
        {
            return false;
        }
    }
    return true;
}

bool BackupMgr::RetrieveFile( const t_char *relPath, const t_char *destPath )
{
    if (NULL == mFi)
    {
        return false;
    }
    assert(NULL != mFi);

    int pos = mFi->Find(relPath);
    if (pos < 0)
    {
        return false;
    }

    FileEntry *fe = mFi->Get(pos);
    if (NULL == fe)
    {
        return false;
    }

    return RetrieveFile(fe->oid, destPath);
}

t_string BackupMgr::GetTagPath()
{
    return DirUtil::MakeFilePath(mBackupDir, L"tags");
}

bool BackupMgr::SetTag( const t_char *tagName )
{
    // Generate oid
    object_id oid;
    if (!GetTagOid(tagName, oid))
    {
        return false;
    }

    DirectBlob db(mBackupDir.c_str(), oid);
    const size_t bufLen = db.GetBlobSize();
    if (0 == bufLen)
    {
        return false;
    }

    char *blobBuf = new char[bufLen];
    if (db.Read(blobBuf, bufLen) == bufLen)
    {
        if (NULL != mFi)
        {
            delete mFi;
        }
        mFi = new FileIndex(blobBuf, bufLen);
    }
    delete []blobBuf;
    return true;
}

bool BackupMgr::GetTagOid( const t_string &tagName, object_id &oid )
{
    char tagOidBuf[HASH_SHA1_STRING_LEN+1];
    tagOidBuf[HASH_SHA1_STRING_LEN] = '\0';

    t_string tagFilePath = DirUtil::MakeFilePath(GetTagPath(), tagName);
    FileBuf fb(tagFilePath.c_str(), FileBuf::FILE_MODE_READ);
    if (HASH_SHA1_STRING_LEN == fb.Read(tagOidBuf, HASH_SHA1_STRING_LEN))
    {
        oid = Sha1Hash::StringToOid(tagOidBuf);
        return true;
    }
    else
    {
        return false;
    }
}

bool BackupMgr::GetTagOid( const t_string &tagName, std::string &oid )
{
    char tagOidBuf[HASH_SHA1_STRING_LEN+1];
    tagOidBuf[HASH_SHA1_STRING_LEN] = '\0';

    t_string tagFilePath = DirUtil::MakeFilePath(GetTagPath(), tagName);
    FileBuf fb(tagFilePath.c_str(), FileBuf::FILE_MODE_READ);
    if (HASH_SHA1_STRING_LEN == fb.Read(tagOidBuf, HASH_SHA1_STRING_LEN))
    {
        oid = tagOidBuf;
        return true;
    }
    else
    {
        return false;
    }
}

IBlob* BlobObjectFactory::GetDirectBlob()
{
    //return new DirectBlob();
    return NULL;
}

}
