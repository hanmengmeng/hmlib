#include "hm_xml_parser.h"
#include "rapidxml_iterators.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"

#include <iostream>
#include <tchar.h>

using namespace rapidxml;
using namespace std;

namespace hm
{

bool operator== (const NodeAttribute &na1, const NodeAttribute &na2)
{
    return na1.attrName == na2.attrName
        && na2.attrValue == na2.attrValue;
}

bool operator== (const Node &n1, const Node &n2)
{
    if (n1.nodeName == n2.nodeName 
        && n1.nodeValue == n2.nodeValue
        && n1.attrs.size() == n2.attrs.size())
    {
        return std::equal(n1.attrs.begin(), n1.attrs.end(), n2.attrs.begin());
    }
    else
    {
        return false;
    }
}

class XmlParserImpl : public IXmlParser
{
public:
    XmlParserImpl(const hm_string &path, const char *rootNodeName = NULL);
    ~XmlParserImpl();

    virtual bool AddNode(const char *name, const char *value, const std::vector<NodeAttribute> &attrs);
    virtual bool AddNode(const Node &n);
    virtual bool ReadNodes(std::vector<Node> &nodes);
    virtual bool RemoveNode(const Node &n);

private:
    Node GetNode(xml_node<> *n);

private:
    rapidxml::xml_document<> mDoc;
    rapidxml::xml_node<> *mRootNode;
    bool mHasModify;
    hm_string mXmlPath;
    std::vector<char> mFileBuffer;
};

XmlParserImpl::XmlParserImpl(const hm_string &path, const char *rootNodeName)
{
    if (_taccess(path.c_str(), 0) == 0)
    {
        std::ifstream labelXml(path);
        mFileBuffer.insert(mFileBuffer.begin(), (istreambuf_iterator<char>(labelXml)), istreambuf_iterator<char>());
        mFileBuffer.push_back('\0');
        mDoc.parse<0>(&mFileBuffer[0]);

        if (NULL == rootNodeName)
        {
            mRootNode = mDoc.first_node();
        }
        else
        {
            mRootNode = mDoc.first_node(rootNodeName);
        }
    }
    else
    {
        mRootNode = mDoc.allocate_node(node_element, mDoc.allocate_string(rootNodeName));
        mDoc.append_node(mRootNode);
    }
    assert(mRootNode != NULL);
    mXmlPath = path;
    mHasModify = false;
}

XmlParserImpl::~XmlParserImpl()
{
    if (mHasModify)
    {
        // Create declaration header
        xml_node<>* decl = mDoc.allocate_node(node_declaration);
        decl->append_attribute(mDoc.allocate_attribute("version", "1.0"));
        decl->append_attribute(mDoc.allocate_attribute("encoding", "UTF-8"));
        mDoc.prepend_node(decl);

        std::ofstream myfile;
        myfile.open(mXmlPath.c_str());
        myfile << mDoc;
    }
}

bool XmlParserImpl::AddNode( const char *name, const char *value, const std::vector<NodeAttribute> &attrs )
{
    if (NULL == name)
    {
        return false;
    }
    if (NULL == value)
    {
        value = "";
    }
    // Create node
    xml_node<> *label = mDoc.allocate_node(node_element, mDoc.allocate_string(name), mDoc.allocate_string(value));
    mRootNode->append_node(label);
    for (size_t i = 0; i < attrs.size(); i++)
    {
        xml_attribute<> *att = mDoc.allocate_attribute(mDoc.allocate_string(attrs.at(i).attrName.c_str()), 
            mDoc.allocate_string(attrs.at(i).attrValue.c_str()));
        label->append_attribute(att);
    }

    mHasModify = true;
    return true;
}

bool XmlParserImpl::AddNode( const Node &n )
{
    return AddNode(n.nodeName.c_str(), n.nodeValue.c_str(), n.attrs);
}

bool XmlParserImpl::ReadNodes( std::vector<Node> &nodes )
{
    if (NULL == mRootNode)
    {
        return false;
    }

    for (xml_node<> *node = mRootNode->first_node(); node != NULL; node = node->next_sibling())
    {
        nodes.push_back(GetNode(node));
    }

    return true;
}

bool XmlParserImpl::RemoveNode( const Node &n )
{
    if (NULL == mRootNode)
    {
        return false;
    }
    for (xml_node<> *node = mRootNode->first_node(); node != NULL; node = node->next_sibling())
    {
        if (n == GetNode(node))
        {
            mRootNode->remove_node(node);
            mHasModify = true;
            break;
        }
    }

    return true;
}

Node XmlParserImpl::GetNode( xml_node<> *node )
{
    Node val;
    val.nodeName = node->name();
    if (node->value() == NULL)
    {
        val.nodeValue = "";
    }
    else
    {
        val.nodeValue = node->value();
    }

    for (xml_attribute<> *att = node->first_attribute(); att != NULL; att = att->next_attribute())
    {
        NodeAttribute na;
        if (att->name() == NULL)
        {
            na.attrName = "";
        }
        else
        {
            na.attrName = att->name();
        }

        if (att->value() == NULL)
        {
            na.attrValue = "";
        }
        else
        {
            na.attrValue = att->value();
        }
        val.attrs.push_back(na);
    }

    return val;
}

XmlParser::XmlParser( const hm_string &path, const char *rootNodeName )
{
    impl = new XmlParserImpl(path, rootNodeName);
}

XmlParser::~XmlParser()
{
    delete impl;
}

bool XmlParser::AddNode( const char *name, const char *value, const std::vector<NodeAttribute> &attrs )
{
    return impl->AddNode(name, value, attrs);
}

bool XmlParser::AddNode( const Node &n )
{
    return impl->AddNode(n);
}

bool XmlParser::ReadNodes( std::vector<Node> &nodes )
{
    return impl->ReadNodes(nodes);
}

bool XmlParser::RemoveNode( const Node &n )
{
    return impl->RemoveNode(n);
}

}

