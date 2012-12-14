#include "hm_dir.h"
#include <Windows.h>
#include <tchar.h>

#define DEFAULT_PATH_SPLITER _T('\\')

namespace hm
{

bool DirUtil::EnumFiles( const hm_string &path, std::vector<hm_string> &fileList )
{
    return EnumFilesOrDir(path, fileList, ENUM_FILE);
}

bool DirUtil::EnumDirs( const hm_string &path, std::vector<hm_string> &dirList )
{
    return EnumFilesOrDir(path, dirList, ENUM_DIR);
}

bool DirUtil::EnumAllFiles( const hm_string &path, std::vector<hm_string> &fileList )
{
    if (!EnumFiles(path, fileList))
    {
        return false;
    }

    std::vector<hm_string> dirList;
    if (!EnumDirs(path, dirList))
    {
        return false;
    }

    bool ret = true;
    for (size_t i = 0; i < dirList.size(); i++)
    {
        if (!EnumAllFiles(dirList.at(i), fileList))
        {
            ret = false;
            break;
        }
    }
    return ret;
}

bool DirUtil::DeleteDirectory( const hm_string &path )
{
    SHFILEOPSTRUCT FileOp;
    ZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));

    hm_string strFromPath = MakePathRegular(path);
    // this string must be double-null terminated
    strFromPath.append(1, _T('\0'));

    FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;
    FileOp.hNameMappings = NULL;
    FileOp.hwnd = NULL;
    FileOp.lpszProgressTitle = NULL;
    FileOp.pFrom = strFromPath.c_str();
    FileOp.pTo = NULL;
    FileOp.wFunc = FO_DELETE;

    return SHFileOperation(&FileOp) == 0;
}

bool DirUtil::MoveDirectory( const hm_string &fromPath, const hm_string &toPath )
{
    SHFILEOPSTRUCT FileOp;
    ZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));

    hm_string strFromPath = MakePathRegular(fromPath);
    // this string must be double-null terminated
    strFromPath.append(1, _T('\0'));

    hm_string strToPath = MakePathRegular(toPath);
    // this string must be double-null terminated
    strToPath.append(1, _T('\0'));

    FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;
    FileOp.hNameMappings = NULL;
    FileOp.hwnd = NULL;
    FileOp.lpszProgressTitle = NULL;
    FileOp.pFrom = strFromPath.c_str();
    FileOp.pTo = strToPath.c_str();
    FileOp.wFunc = FO_MOVE;

    return SHFileOperation(&FileOp) == 0;
}

bool DirUtil::IsDirectoryExist( const hm_string &path )
{
    hm_string regularPath = MakePathRegular(path);
    DWORD dwAttr = ::GetFileAttributes(regularPath.c_str());
    return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

bool DirUtil::IsFileExist( const hm_string &path )
{
    hm_string regularPath = MakePathRegular(path);
    DWORD dwAttributes = ::GetFileAttributes(regularPath.c_str());
    return (dwAttributes != INVALID_FILE_ATTRIBUTES) && (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool DirUtil::CreateParentDirectory( const hm_string &path )
{
    hm_string regularPath = MakePathRegular(path);
    for (size_t i = 0; i < regularPath.length(); i++)
    {
        if (DEFAULT_PATH_SPLITER == regularPath.at(i))
        {
            hm_string p = regularPath.substr(0, i);
            if (!IsDirectoryExist(p))
            {
                if (!::CreateDirectory(p.c_str(), NULL))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

hm_string DirUtil::MakePathRegular( const hm_string &path )
{
    hm_string ret = path;
    size_t count = ret.length();
    for (size_t i = 0; i < count; i++)
    {
        if (_T('/') == ret[i])
        {
            ret[i] = DEFAULT_PATH_SPLITER;
        }
    }
    return ret;
}

bool DirUtil::EnumFilesOrDir( const hm_string &path, std::vector<hm_string> &fileList, int enumType)
{
    if (path.empty())
    {
        return false;
    }

    hm_string strDirPath = MakePathRegular(path);
    if (strDirPath.find_last_of(DEFAULT_PATH_SPLITER) != strDirPath.length() - 1)
    {
        strDirPath.push_back(DEFAULT_PATH_SPLITER);
    }

    hm_string filter = strDirPath;
    filter.append(_T("*"));

    WIN32_FIND_DATA fd;
    HANDLE  hFind = FindFirstFile( filter.c_str(), &fd );
    if ( hFind == INVALID_HANDLE_VALUE ){
        return false;
    }

    do
    {
        if (ENUM_FILE & enumType)
        {
            if (!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
            {
                hm_string fp = strDirPath;
                fp.append(fd.cFileName);
                fileList.push_back(fp);
            }
        }

        if (ENUM_DIR & enumType)
        {
            if ( _tcscmp(fd.cFileName, _T(".")) != 0
                && _tcscmp(fd.cFileName, _T("..")) != 0
                && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                hm_string fp = strDirPath;
                fp.append(fd.cFileName);
                fileList.push_back(fp);
            }
        }

    } while ( FindNextFile( hFind, &fd ) != 0 );

    FindClose( hFind );

    return true;
}


hm::DirUtil::DirUtil()
{
}

hm::DirUtil::~DirUtil()
{
}

} // namespace hm
