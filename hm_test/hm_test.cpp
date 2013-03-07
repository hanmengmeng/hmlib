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
    std::wstring wstr = StringConvert(str.c_str()).ToUtf16();
    LPCTSTR lp = wstr.c_str();
    std::string u8str = StringConvert(str.c_str()).ToUtf8();
    LPCSTR lp2 = u8str.c_str();
}

void TestEnumDir()
{
    std::vector<t_string> fileList;
    DirUtil::EnumFiles(L"D:\\workspace", fileList);

    std::vector<t_string> dirList;
    DirUtil::EnumDirs(L"D:\\workspace", dirList);

    std::vector<t_string> fileList2;
    DirUtil::EnumFilesRecursion(L"D:\\workspace\\apache-log4cxx-0.10.0", fileList2);
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


struct TreeNode
{
    struct TreeNode *left;
    struct TreeNode *right;
    int value;
};

void CreateTempTree(struct TreeNode *node, int depth)
{
    if (depth <= 1)
    {
        return;
    }
    struct TreeNode *left = new struct TreeNode;
    left->left = NULL;
    left->right = NULL;
    left->value = rand()%1000;
    struct TreeNode *right = new struct TreeNode;
    right->left = NULL;
    right->right = NULL;
    right->value = rand()%1000;
    node->left = left;
    node->right = right;

    CreateTempTree(node->left, depth - 1);
    CreateTempTree(node->right, depth - 1);
}

void ReadTreeWide(struct TreeNode *root)
{
    if (0 == root)
    {
        return;
    }

    QueueA<struct TreeNode *> q;
    q.PushBack(root);

    while (q.GetSize() > 0)
    {
        struct TreeNode *tn = q.PopFront();
        printf("%d ", tn->value);
        if (NULL != tn->left)
        {
            q.PushBack(tn->left);
        }
        if (NULL != tn->right)
        {
            q.PushBack(tn->right);
        }
    }
}

void ReadTreeDepth(struct TreeNode *root)
{
    if (NULL == root)
    {
        return;
    }
    Stack<struct TreeNode*> s;
    Stack<struct TreeNode*> s2;
    struct TreeNode *n = root;

    // 后序遍历
    while (NULL != n)
    {
        if (NULL != n->left)
        {
            s.PushBack(n->left);
            n = n->left;
        }
        else if (NULL != n->right)
        {
            s.PushBack(n->right);
            n = n->right;
        }
        else
        {
            n = NULL;
        }
    }

    while (!s.IsEmpty())
    {
        printf("%d ", s.PopUp()->value);
    }

#if 0
    // 前序遍历
    while (!s.IsEmpty())
    {
        n = s.PopUp();
        printf("%d ", n->value);
        if (NULL != n->right)
        {
            s.PushBack(n->right);
        }
        if (NULL != n->left)
        {
            s.PushBack(n->left);
        }
    }
#endif
}

void ReadTreeDepthRecur(struct TreeNode *root)
{
    if (NULL == root)
    {
        return;
    }
    
    ReadTreeDepthRecur(root->left);
    ReadTreeDepthRecur(root->right);
    printf("%d ", root->value);
}

void AddFiles()
{
    BackupMgr bm(L"D:\\testgit3");
    std::vector<t_string> fileList;

    t_string path = L"D:\\备份";
    DirUtil::EnumFiles(path, fileList);
    for (size_t i = 0; i < fileList.size(); i++)
    {
        t_string p = fileList.at(i);
        if (DirUtil::IsDirectoryExist(p))
        {
            bm.AddDir(p.c_str(), p.substr(path.size()+1).c_str());
        }
        else
        {
            bm.AddFile(p.c_str(), p.substr(path.size()+1).c_str());
        }
    }

    bm.Finish("201301242008");
}

void RestoreFiles()
{
    BackupMgr bm(L"D:\\testgit3");
    bm.SetTag(L"201301242008");

    FileEntryList files;
    bm.GetFileList(files);
    
    for (size_t i = 0; i < files.size(); i++)
    {
        t_string outpath = L"D:\\output\\";
        outpath += StringConvert(files.at(i).path).ToUtf16();
        bm.RetrieveFile(files.at(i).oid, outpath.c_str());
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    //AddFiles();
    RestoreFiles();
    return 0;
}

