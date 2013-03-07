#ifndef SAVE_LABEL_H
#define SAVE_LABEL_H

#include <vector>
#include <string>

#include "hm_common.h"

class XmlParserImpl;
namespace hm
{

typedef struct
{
    std::string attrName;
    std::string attrValue;
}NodeAttribute; // XML Node's attribute

typedef struct
{
    std::string nodeName;
    std::string nodeValue;
    std::vector<NodeAttribute> attrs;
}Node; // XML Node

class IXmlParser
{
public:
    virtual ~IXmlParser(){}
    // Add new node to xml file
    virtual bool AddNode(const char *name, const char *value, const std::vector<NodeAttribute> &attrs) = 0;
    virtual bool AddNode(const Node &n) = 0;

    // Read all node from xml file
    virtual bool ReadNodes(std::vector<Node> &nodes) = 0;

    // Remove the first node that matches parameter n
    virtual bool RemoveNode(const Node &n) = 0;
};

class XmlParser : public IXmlParser
{
public:
    XmlParser(const t_string &path, const char *rootNodeName = NULL);
    ~XmlParser();

    virtual bool AddNode(const char *name, const char *value, const std::vector<NodeAttribute> &attrs);
    virtual bool AddNode(const Node &n);
    virtual bool ReadNodes(std::vector<Node> &nodes);
    virtual bool RemoveNode(const Node &n);

private:
    XmlParserImpl *impl;
};

}
#endif

