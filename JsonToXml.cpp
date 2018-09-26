#include "JsonToXml.h"
#include <sstream>
#include <iostream>

JsonToXml::JsonToXml(const std::string &jsonStr)
{
    m_docJson.Parse<0>(jsonStr.c_str());

    if (m_docJson.HasParseError()) {
        printf("parse strMsg error:[%d]\n", __LINE__);
    }
}


void JsonToXml::transferJson2Xml(std::string &str, const std::string& root)
{
    pugi::xml_document xdoc;
    xdoc.load_string("");
    pugi::xml_node childNode = xdoc.append_child(root.c_str());

    Json2XmlAddNode(childNode, m_docJson);

    std::ostringstream oss;
    xdoc.print(oss);
    str = oss.str();
}

void JsonToXml::Json2XmlAddArray(pugi::xml_node value, rapidjson::Value& jsondoc)
{

}

void JsonToXml::Json2XmlAddNode(pugi::xml_node value, rapidjson::Value& jsondoc)
{
    for (JsonMemberIter node = jsondoc.MemberBegin(); node != jsondoc.MemberEnd(); ++node) {
        std::string nodeName = (node->name).GetString();
        rapidjson::Type nodeType = (node->value).GetType();
        rapidjson::Value _Object;

        if (nodeName.find_first_of("@") == 0 && (node->value).GetType() == kStringType) {
            std::string nodeAttr = (node->value).GetString();
            nodeName = nodeName.replace(0, 1, "");
            value.append_attribute(nodeName.c_str()).set_value(nodeAttr.c_str());
            continue;
        }
        else if (nodeName.compare("#text") == 0 && (node->value).GetType() == kStringType) {
            std::string nodeAttr = (node->value).GetString();
            value.text().set(nodeAttr.c_str());
            continue;
        }

        switch (nodeType) {
        case kNullType:
            value.append_child(nodeName.c_str()).append_child(pugi::xml_node_type::node_null);
            break;
        case kObjectType:
            _Object = (node->value).GetObject();
            Json2XmlAddNode(value.append_child(nodeName.c_str()), _Object); // 必须是引用类型
            break;
        case kArrayType:
            for (auto& iter : ((node->value).GetArray())) {
                if (iter.GetType() == rapidjson::kStringType) {
                    value.append_child(nodeName.c_str()).append_child(pugi::xml_node_type::node_pcdata).set_value(iter.GetString());
                }
                else if (iter.GetType() == rapidjson::kNumberType || iter.GetType() == rapidjson::kFalseType || iter.GetType() == rapidjson::kTrueType) {
                    value.append_child(nodeName.c_str()).append_child(pugi::xml_node_type::node_pcdata).set_value(std::to_string(iter.GetInt()).c_str());
                }
                else if (iter.GetType() == rapidjson::kObjectType) {
                    _Object = iter.GetObject();
                    Json2XmlAddNode(value.append_child(nodeName.c_str()), _Object);
                }
            }
            break;
        case kTrueType:
        case kFalseType:
            if ((node->value).GetBool()) {
                value.append_child(nodeName.c_str()).append_child(pugi::xml_node_type::node_pcdata).set_value("true");
            }
            else {
                value.append_child(nodeName.c_str()).append_child(pugi::xml_node_type::node_pcdata).set_value("false");
            }
            break;
        case kStringType:
            value.append_child(nodeName.c_str()).append_child(pugi::xml_node_type::node_pcdata).set_value((node->value).GetString());
            break;
        case kNumberType:
            value.append_child(nodeName.c_str()).append_child(pugi::xml_node_type::node_pcdata).set_value(std::to_string((node->value).GetInt()).c_str());
            break;
        default:
            break;
        }
    }
}
