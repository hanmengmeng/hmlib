#include "hm_dir.h"
#include <Windows.h>
#include <tchar.h>

#define DEFAULT_PATH_SPLITER _T('\\')

namespace hm
{

bool DirUtil::EnumFiles( const t_string &path, std::vector<t_string> &fileList )
{
    return EnumFilesOrDir(path, fileList, ENUM_FILE);
}

bool DirUtil::EnumDirs( const t_string &path, std::vector<t_string> &dirList )
{
    return EnumFilesOrDir(path, dirList, ENUM_DIR);
}

bool DirUtil::EnumAllFiles( const t_string &path, std::vector<t_string> &fileList )
{
    if (!EnumFiles(path, fileList))
    {
        return false;
    }

    std::vector<t_string> dirList;
    if (!EnumDirs(path, dirList))
    {
        return false;
    }

    bool ret = true;
    for (t_size i = 0; i < dirList.size(); i++)
    {
        if (!EnumAllFiles(dirList.at(i), fileList))
        {
            ret = false;
            break;
        }
    }
    return ret;
}

bool DirUtil::DeleteDirectory( const t_string &path )
{
    SHFILEOPSTRUCT FileOp;
    ZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));

    t_string strFromPath = MakePathRegular(path);
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

bool DirUtil::MoveDirectory( const t_string &fromPath, const t_string &toPath )
{
    SHFILEOPSTRUCT FileOp;
    ZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));

    t_string strFromPath = MakePathRegular(fromPath);
    // this string must be double-null terminated
    strFromPath.append(1, _T('\0'));

    t_string strToPath = MakePathRegular(toPath);
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

bool DirUtil::IsDirectoryExist( const t_string &path )
{
    t_string regularPath = MakePathRegular(path);
    DWORD dwAttr = ::GetFileAttributes(regularPath.c_str());
    return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

bool DirUtil::IsFileExist( const t_string &path )
{
    t_string regularPath = MakePathRegular(path);
    DWORD dwAttributes = ::GetFileAttributes(regularPath.c_str());
    return (dwAttributes != INVALID_FILE_ATTRIBUTES) && (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool DirUtil::CreateParentDirectory( const t_string &path )
{
    t_string regularPath = MakePathRegular(path);
    for (t_size i = 0; i < regularPath.length(); i++)
    {
        if (DEFAULT_PATH_SPLITER == regularPath.at(i))
        {
            t_string p = regularPath.substr(0, i);
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

t_string DirUtil::MakePathRegular( const t_string &path )
{
    t_string ret = path;
    t_size count = ret.length();
    for (t_size i = 0; i < count; i++)
    {
        if (_T('/') == ret[i])
        {
            ret[i] = DEFAULT_PATH_SPLITER;
        }
    }
    return ret;
}

bool DirUtil::EnumFilesOrDir( const t_string &path, std::vector<t_string> &fileList, int enumType)
{
    if (path.empty())
    {
        return false;
    }

    t_string strDirPath = MakePathRegular(path);
    if (strDirPath.find_last_of(DEFAULT_PATH_SPLITER) != strDirPath.length() - 1)
    {
        strDirPath.push_back(DEFAULT_PATH_SPLITER);
    }

    t_string filter = strDirPath;
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
                t_string fp = strDirPath;
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
                t_string fp = strDirPath;
                fp.append(fd.cFileName);
                fileList.push_back(fp);
            }
        }

    } while ( FindNextFile( hFind, &fd ) != 0 );

    FindClose( hFind );

    return true;
}

bool DirUtil::MakeTempFile( t_string &dirPath )
{
    t_string regularPath = MakePathRegular(dirPath);
    if ( regularPath.length() > 0 
        && regularPath.at(regularPath.length() - 1) != DEFAULT_PATH_SPLITER)
    {
        regularPath.push_back(DEFAULT_PATH_SPLITER);
    }
    regularPath.append(_T("fnXXXXXX"));

    TCHAR szPath[MAX_PATH];
    memset(szPath, 0, MAX_PATH);
    _tcsncpy(szPath, regularPath.c_str(), regularPath.size());
    if (_tmktemp_s(szPath, _tcslen(szPath)+1) != 0)
    {
        return false;
    }
    dirPath = szPath;

    return true;
}

bool DirUtil::MoveFile( const t_string &fromPath, const t_string &toPath )
{
    return MoveDirectory(fromPath, toPath);
}

t_string DirUtil::MakeFilePath( const t_string &dirPath, const t_string &fileName )
{
    t_string strDirPath = MakePathRegular(dirPath);
    if (strDirPath.find_last_of(DEFAULT_PATH_SPLITER) != strDirPath.length() - 1)
    {
        strDirPath.push_back(DEFAULT_PATH_SPLITER);
    }
    strDirPath.append(fileName);
    return strDirPath;
}

bool DirUtil::DeleteFileIfExist( const t_string &filePath )
{
    if (IsFileExist(filePath))
    {
        return ::DeleteFileW(filePath.c_str());
    }
    else
    {
        return true;
    }
}

bool DirUtil::CreateDirectory( const t_string &dirPath )
{
    if (!CreateParentDirectory(dirPath))
    {
        return false;
    }
    return ::CreateDirectory(dirPath.c_str(), NULL) == TRUE;
}


hm::DirUtil::DirUtil()
{
}

hm::DirUtil::~DirUtil()
{
}

} // namespace hm
