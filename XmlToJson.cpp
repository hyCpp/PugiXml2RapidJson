#include "XmlToJson.h"
#include <map>
#include "../externals/pugixml/pugixml.cpp"

XmlToJson::XmlToJson(const std::string& xmlStr)
{
    if (!m_dom.load_string(xmlStr.c_str(),  pugi::parse_full)) {
        printf("m_dom.load_string() is fail !");
    }
}

void XmlToJson::transferXml2Json(std::string& str, const std::string& node)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& _alloca = document.GetAllocator();

    pugi::xml_node rootNode = m_dom.select_node(("//" + node).c_str()).node();
    Xml2JsonAddAttr(rootNode, document, _alloca); // add attribute of root node
    for (pugi::xml_node node = rootNode.first_child(); node; node = node.next_sibling()) {
        rapidjson::Value valueChild;
        valueChild.SetObject();
        Xml2JsonAddNode(node, valueChild, _alloca);

        /**
         *  不能直接使用StringRef(xxx.c_str());StringRef是引用转移，也就是把指针指向了真正内容所在的内存区域，xxx是局部变量当析构时指针指向未知区域导致乱码(\0000,\u000这种)，
         *  Value().SetString(curNodeAttrName.c_str(), curNodeAttrName.size(), _alloc)将用alloc分配内存使用copy的变量处理
         */
        if (strcmp(node.name(), "")) {
            document.AddMember(rapidjson::Value().SetString(node.name(), _alloca), valueChild, _alloca);
        }
        else {
            document.AddMember("#text", valueChild, _alloca);
        }
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    str = buffer.GetString();
}

void XmlToJson::Xml2JsonAddNode(pugi::xml_node node, rapidjson::Value& value, rapidjson::Document::AllocatorType& _alloc)
{
    rapidjson::Value valueChild;
    valueChild.SetObject();
    value.SetObject();
    pugi::xml_node_type curNodeType = node.type();
    pugi::xml_node_type childNodeType = node.first_child().type();

//    printf("node name = [%s] type = [%d]\n", node.name(), curNodeType);
    switch (curNodeType) {
    case pugi::node_pi:
    case pugi::node_declaration:
    case pugi::node_doctype:
    case pugi::node_comment:
        return;
    case pugi::node_pcdata:
    case pugi::node_cdata:
        if (node.value()) {
            value.SetString(node.value(), strlen(node.value()), _alloc);
        }
        break;
    case pugi::node_element:
        Xml2JsonAddAttr(node, value, _alloc);
        if (childNodeType == pugi::node_null) { // i.e : <hy/> or <hy></hy>
            value.SetNull();
            return;
        }
        if (childNodeCount(node) == 1 && childNodeType == pugi::node_pcdata) {
            singleNode(node, value, _alloc);
            return;
        }
        else if (childNodeType == pugi::node_element) {
            std::map<std::string, int> nameCountMap;
            for(pugi::xml_node chNode = node.first_child(); chNode; chNode = chNode.next_sibling()) {
                std::string current_name;
                rapidjson::Value jn, jv;
                if(chNode.type() == pugi::node_pcdata || chNode.type() == pugi::node_cdata) {
                    current_name = "#text";
                    nameCountMap[current_name]++;
                }
                else if (chNode.type() == pugi::node_element){
                    current_name = chNode.name();
                    nameCountMap[current_name]++;
                }
                Xml2JsonAddNode(chNode, valueChild, _alloc);

                if (nameCountMap[current_name] > 1 && !current_name.empty()) {
                    xml2JsonAddArray(current_name, value, valueChild, _alloc);
                }
                else {
                    jn.SetString(current_name.c_str(), _alloc);
                    value.AddMember(jn, valueChild, _alloc);
                }
            }
        }
        break;
    default:
        break;
    }
}

void XmlToJson::singleNode(pugi::xml_node node, rapidjson::Value& value, rapidjson::Document::AllocatorType& _alloc)
{
    if (node.first_attribute() != nullptr) { // i.e : <hy attr="xxx">text</hy>
        value.AddMember(rapidjson::Value().SetString("#text", _alloc),
                        rapidjson::Value().SetString(node.child_value(), _alloc), _alloc);
    }
    else { // i.e : <hy>text</hy>
        value.SetString(node.first_child().value(), _alloc);
    }
}

void XmlToJson::xml2JsonAddArray(const std::string& name, rapidjson::Value& value, rapidjson::Value& childValue, rapidjson::Document::AllocatorType& _alloc)
{
    rapidjson::Value valueTarget;
    rapidjson::Value jn;
    jn.SetString(name.c_str(), _alloc);
    valueTarget = value.FindMember(name.c_str())->value;

    if(valueTarget.IsArray()) {
        valueTarget.PushBack(childValue, _alloc);
        value.RemoveMember(name.c_str());
        value.AddMember(jn, valueTarget, _alloc);
    }
    else {
        rapidjson::Value valueArray;
        valueArray.SetArray();
        valueArray.PushBack(valueTarget, _alloc);
        valueArray.PushBack(childValue, _alloc);
        value.RemoveMember(name.c_str());
        value.AddMember(jn, valueArray, _alloc);
    }
}

void XmlToJson::Xml2JsonAddAttr(pugi::xml_node node, rapidjson::Value& value, rapidjson::Document::AllocatorType& _alloc)
{
    for (pugi::xml_node::attribute_iterator attr = node.attributes_begin(); attr != node.attributes_end(); ++attr) {
        std::string attrName = attr->name();
        std::string attrVal = attr->value();

        if (attrName.compare("") == 0 || attrVal.compare("") == 0) {
            printf("Xml2JsonAddAttr error, node attribute is empty \n");
        }
        else {
            attrName = "@" + attrName;
            value.AddMember(rapidjson::Value().SetString(attrName.c_str(), attrName.size(), _alloc),
                          rapidjson::Value().SetString(attrVal.c_str(), attrVal.size(), _alloc), _alloc);
            printf("attrName = [%s], attrVal = [%s]\n", attrName.c_str(), attrVal.c_str());
        }
    }
}

int XmlToJson::childNodeCount(pugi::xml_node node)
{
    int count = 0;
    for (pugi::xml_node chNode = node.first_child(); chNode; chNode = chNode.next_sibling()) {
        count++;
    }

    return count;
}

