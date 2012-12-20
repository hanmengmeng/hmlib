// hm_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

void TestAddNode()
{
    XmlParser xp(L"D:/test.xml", "testhm");
    std::vector<NodeAttribute> vattrs;
    NodeAttribute na1 = {"test", "222"};
    vattrs.push_back(na1);
    NodeAttribute na2 = {"test222", "3333"};
    vattrs.push_back(na2);
    std::string str = "node1";

    std::string value = StringConvert("中国AB").ToUtf8();
    xp.AddNode(str.c_str(), value.c_str(), vattrs);
}

void TestReadNode()
{
    XmlParser xp(L"D:/test.xml", "testhm1");
    std::vector<Node> vn;
    xp.ReadNodes(vn);
}

void TestRemoveNode()
{
    XmlParser xp(L"D:/test.xml");
    Node node;
    node.nodeName = "node1";
    node.nodeValue = "245";
    NodeAttribute na = {"test", "222"};
    node.attrs.push_back(na);
    NodeAttribute na2 = {"test222", "333"};
    node.attrs.push_back(na2);
    xp.RemoveNode(node);
}

void TestStringConvert()
{
    std::string str = "中国ab";
    std::wstring wstr = StringConvert(str.c_str()).ToUnicode();
    LPCTSTR lp = wstr.c_str();
    std::string u8str = StringConvert(str.c_str()).ToUtf8();
    LPCSTR lp2 = u8str.c_str();
}

void TestEnumDir()
{
    std::vector<hm_string> fileList;
    DirUtil::EnumFiles(L"D:\\workspace", fileList);

    std::vector<hm_string> dirList;
    DirUtil::EnumDirs(L"D:\\workspace", dirList);

    std::vector<hm_string> fileList2;
    DirUtil::EnumAllFiles(L"D:\\workspace\\apache-log4cxx-0.10.0", fileList2);
}

void TestDeleteDirectory()
{
    DirUtil::DeleteDirectory(L"D:\\test_git_workspace22");
}

void TestMoveDirectory()
{
    DirUtil::MoveDirectory(L"D:\\test_git_workspace33", L"D:\\test_git_workspace22");
}

void TestMakeDirectory()
{
    DirUtil::CreateParentDirectory(L"D:\\123\\456\\789");
}

void TestReadFile()
{
    FileBuf fb(L"D:/skin.xml", FileBuf::FILE_MODE_READ);
    char buf[4096];
    int size = fb.Read(buf, 4096);
    int fs = fb.GetFileSize();
}

int _tmain(int argc, _TCHAR* argv[])
{
    BackupMgr bm(L"D:\\testgit3");
    bm.AddFile(NULL, NULL);
    bm.AddFile(NULL, NULL);
    object_id id;
    bm.Finish(id);
	return 0;
}

