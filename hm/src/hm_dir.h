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

    static bool EnumFiles(const hm_string &path, std::vector<hm_string> &fileList);
    static bool EnumDirs(const hm_string &path, std::vector<hm_string> &dirList);
    static bool EnumAllFiles(const hm_string &path, std::vector<hm_string> &fileList);
    static bool DeleteDirectory(const hm_string &path);
    static bool MoveDirectory(const hm_string &fromPath, const hm_string &toPath);
    static bool IsDirectoryExist(const hm_string &path);
    static bool IsFileExist(const hm_string &path);
    static bool CreateParentDirectory(const hm_string &path);

    // Make path to use backslash
    static hm_string MakePathRegular(const hm_string &path);

private:
    enum
    {
        ENUM_FILE = 1 << 1, // Only enumerate files under the path
        ENUM_DIR = 1 << 2,  // Only enumerate directory under the path
    };
    static bool EnumFilesOrDir(const hm_string &path, std::vector<hm_string> &fileList, int enumType);
};
}

#endif
