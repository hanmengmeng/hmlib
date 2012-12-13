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

int _tmain(int argc, _TCHAR* argv[])
{
    TestAddNode();
	return 0;
}

