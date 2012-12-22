#ifndef HM_DIR_H
#define HM_DIR_H

#include <vector>
#include "hm_common.h"

namespace hm
{
class DirUtil
{
public:
    DirUtil();
    ~DirUtil();

    static bool EnumFiles(const t_string &path, std::vector<t_string> &fileList);
    static bool EnumDirs(const t_string &path, std::vector<t_string> &dirList);
    static bool EnumAllFiles(const t_string &path, std::vector<t_string> &fileList);
    static bool DeleteDirectory(const t_string &path);
    static bool MoveDirectory(const t_string &fromPath, const t_string &toPath);
    static bool IsDirectoryExist(const t_string &path);
    static bool IsFileExist(const t_string &path);
    static bool CreateParentDirectory(const t_string &path);
    static bool MakeTempFile(t_string &dirPath);
    static bool MoveFile(const t_string &fromPath, const t_string &toPath);
    static bool DeleteFile(const t_string &filePath);
    static t_string MakeFilePath(const t_string &dirPath, const t_string &fileName);

    // Make path to use backslash
    static t_string MakePathRegular(const t_string &path);

private:
    enum
    {
        ENUM_FILE = 1 << 1, // Only enumerate files under the path
        ENUM_DIR = 1 << 2,  // Only enumerate directory under the path
    };
    static bool EnumFilesOrDir(const t_string &path, std::vector<t_string> &fileList, int enumType);
};
}

#endif
