#ifndef JSONTOXML
#define JSONTOXML

#include <string>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "../externals/pugixml/pugixml.hpp"

using namespace rapidjson;

class JsonToXml
{
public:
    typedef GenericMemberIterator<false, UTF8<>, MemoryPoolAllocator<> >::Iterator JsonMemberIter;
    JsonToXml(const std::string& jsonStr);

    void transferJson2Xml(std::string& str, const std::string& root);

private:
    void Json2XmlAddNode(pugi::xml_node value, rapidjson::Value& node);
    void Json2XmlAddArray(pugi::xml_node value, rapidjson::Value& jsondoc);


private:
    rapidjson::Document m_docJson;
};

#endif // JSONTOXML

