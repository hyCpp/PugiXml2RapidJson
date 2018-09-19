#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <fstream>
#include <string>
#include <stdio.h>
#include "../externals/pugixml/pugixml.hpp"
#include "../externals/pugixml/pugixml.cpp"
#include <map>
#include <vector>

using namespace std;
using namespace rapidjson;
using namespace pugi;

Document _doc;

bool LoadJsonFile(const string &path)
{
    ifstream _file(path.c_str());
    if (!_file.is_open()) {
        printf("open DictationConfig.json File failed !\n");
        return false;
    }

    IStreamWrapper _isw(_file);
    EncodedInputStream<UTF8<>, IStreamWrapper> eis(_isw);
    _doc.ParseStream<0, UTF8<> >(eis);

    _file.close();
    return true;
}

bool JsonTest()
{
    if (!LoadJsonFile("xxxx")) {
        printf("Load Json File failed !\n");
    }

    if (_doc["Region"].IsNull()) {
        printf("get request url failed !");
        return false;
    }

    if (_doc["Setting"].IsNull()) {
        printf("get request url param failed !");
        return false;
    }

    if (_doc["RequestHeaders"].IsNull()) {
        printf("get request url param failed !");
        return false;
    }

    printf("url [%s]\n", _doc["Region"]["WestUS"].GetString());
    printf("language [%s]\n", _doc["Setting"]["language"].GetString());
    printf("format [%s]\n", _doc["Setting"]["format"].GetString());
    printf("Transfer-Encoding [%s]\n", _doc["RequestHeaders"]["Transfer-Encoding"].GetString());
    printf("Subscription [%s]\n", _doc["RequestHeaders"]["Ocp-Apim-Subscription-Key"].GetString());
    printf("Content-type [%s]\n", _doc["RequestHeaders"]["Content-type"].GetString());
    return true;
}

const std::string xmldisplay("<display agent=\"Navi\" content=\"SearchResult\" errcode=\"0\">"\
                            "<count>0</count>"\
                                 "<items>"\
                                        "<item>"\
                                               "<type>poi</type>"\
                                               "<poitype>poitype2</poitype>"\
                                               "<poiid>poiid3</poiid>"\
                                               "<poiname>poiname4</poiname>"\
                                               "<poilon>1235</poilon>"\
                                               "<poilat>9981</poilat>"\
                                               "<tel>tel7</tel>"\
                                               "<address>address8</address>"\
                                               "<linkid>linkid9</linkid>"\
                                               "<distance>distance10</distance>"\
                                        "</item>"\
                                        "<item>"\
                                                "<type>song</type>"\
                                                "<songid>11</songid>"\
                                                "<songname>songname12</songname>"\
                                                "<albumid>albumid13</albumid>"\
                                                "<albumname>albumname14</albumname>"\
                                                "<artistid>artistid15</artistid>"\
                                                "<artistname>artistname16</artistname>"\
                                                "<sourceid>sourceid17</sourceid>"\
                                        "</item>"\
                                        "<item>"\
                                               "<type>phone</type>"\
                                               "<contactid>contactid</contactid>"\
                                               "<firstname>firstname</firstname>"\
                                               "<lastname>lastname</lastname>"\
                                               "<phonetype1>phonetype1</phonetype1>"\
                                               "<phonenumber1>phonenumber1</phonenumber1>"\
                                               "<phonetype2>phonetype2</phonetype2>"\
                                               "<phonenumber2>phonenumber2</phonenumber2>"\
                                               "<phonetype3>phonetype3</phonetype3>"\
                                               "<phonenumber3>phonenumber3</phonenumber3>"\
                                               "<phonetype4>phonetype4</phonetype4>"\
                                               "<phonenumber4>phonenumber4</phonenumber4>"\
                                       "</item>"\
                                "</items>"\
                        "</display>");

const std::string XML_TO_JSON("<action domainType=\"tts\" op=\"onStartSpeak\">" \
                                "test"
                                "<id>10</id>" \
                                "<e></e>" \
                                "<type attr=\"test\">updatehints</type>" \
                                "<params>" \
                                    "<scenario>DISPLAY_HINTS_ONLY</scenario>" \
                                    "<hint>aa</hint>" \
                                    "<hint>bb</hint>" \
                                    "<data>世纪汇广场A座</data>" \
                                    "<longitude huyang = \"huyang\">12.45</longitude>" \
                                    "<latitude>142.68</latitude>" \
                                    "<data>世纪汇广场B座</data>" \
                                    "<longitude>67.8</longitude>" \
                                    "<latitude>95.67</latitude>" \
                                "</params>" \
                                "<hello></hello>" \
                            "</action>");


void Xml2JsonAddAttr(pugi::xml_node node, rapidjson::Value& value, Document::AllocatorType& _alloc)
{
    for (pugi::xml_node::attribute_iterator attr = node.attributes_begin(); attr != node.attributes_end(); ++attr) {
        std::string attrName = attr->name();
        std::string attrVal = attr->value();

        if (attrName.compare("") == 0 || attrVal.compare("") == 0) {
            printf("Xml2JsonAddAttr error, node attribute is empty \n");
        }
        else {
            attrName = "@" + attrName;
            value.AddMember(Value().SetString(attrName.c_str(), attrName.size(), _alloc),
                          Value().SetString(attrVal.c_str(), attrVal.size(), _alloc), _alloc);
            printf("attrName = [%s], attrVal = [%s]\n", attrName.c_str(), attrVal.c_str());
        }
    }
}

void xml2JsonAddArray(const std::string& name, rapidjson::Value& value, rapidjson::Value& childValue, Document::AllocatorType& _alloc)
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

int childNodeCount(pugi::xml_node node)
{
    int count = 0;
    for (pugi::xml_node chNode = node.first_child(); chNode; chNode = chNode.next_sibling()) {
        count++;
    }

    return count;
}

void singleNode(pugi::xml_node node, rapidjson::Value& value, Document::AllocatorType& _alloc) {

    if (node.first_attribute() != nullptr) { // i.e : <hy attr="xxx">text</hy>
        Xml2JsonAddAttr(node, value, _alloc);
        value.AddMember(Value().SetString("#text", _alloc),
                        Value().SetString(node.child_value(), _alloc), _alloc);
    }
    else { // i.e : <hy>text</hy>
        value.SetString(node.first_child().value(), _alloc);
    }
}

void Xml2JsonAddNode(pugi::xml_node node, rapidjson::Value& value, Document::AllocatorType& _alloc)
{
    rapidjson::Value valueChild;
    valueChild.SetObject();
    value.SetObject();
    pugi::xml_node_type curNodeType = node.type();
    pugi::xml_node_type childNodeType = node.first_child().type();

    printf("node name = [%s] type = [%d]\n", node.name(), curNodeType);
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
        if (childNodeType == pugi::node_null) { // i.e : <hy/> or <hy></hy>
            value.SetNull();
            return;
        }
        if (childNodeCount(node) == 1 && childNodeType == pugi::node_pcdata) {
            singleNode(node, value, _alloc);
            return;
        }
        else if (childNodeType == pugi::node_element) {
            Xml2JsonAddAttr(node, value, _alloc);
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

std::string transferXml2Json(const std::string& str, const std::string& node)
{
    Document document;
    document.SetObject();
    Document::AllocatorType& _alloca = document.GetAllocator();

    pugi::xml_document xdom;
    if (!xdom.load_string(str.c_str(),  pugi::parse_full)) {
        printf("m_dom.load_string() is fail !");
    }

    pugi::xml_node rootNode = xdom.select_node(("//" + node).c_str()).node();
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
            document.AddMember(Value().SetString(node.name(), _alloca), valueChild, _alloca);
        }
        else {
            document.AddMember("#text", valueChild, _alloca);
        }
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}


int main()
{
    printf("json = %s\n", transferXml2Json(XML_TO_JSON, "display").c_str());
}
