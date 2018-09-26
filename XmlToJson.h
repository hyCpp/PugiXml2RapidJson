#ifndef XMLTOJSON_H
#define XMLTOJSON_H

#include <string>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "../externals/pugixml/pugixml.hpp"


class XmlToJson
{
public:
    XmlToJson(const std::string& xmlStr);

    void transferXml2Json(std::string& str, const std::string& node);

private:
    void Xml2JsonAddNode(pugi::xml_node node, rapidjson::Value& value, rapidjson::Document::AllocatorType& _alloc);
    void singleNode(pugi::xml_node node, rapidjson::Value& value, rapidjson::Document::AllocatorType& _alloc);
    void xml2JsonAddArray(const std::string& name, rapidjson::Value& value, rapidjson::Value& childValue, rapidjson::Document::AllocatorType& _alloc);
    void Xml2JsonAddAttr(pugi::xml_node node, rapidjson::Value& value, rapidjson::Document::AllocatorType& _alloc);
    int childNodeCount(pugi::xml_node node);

private:
    pugi::xml_document m_dom;
};

#endif // XMLTOJSON_H

