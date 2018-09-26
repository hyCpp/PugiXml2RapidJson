#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <fstream>
#include <string>
#include <stdio.h>
#include <map>
#include "XmlToJson.h"
#include "JsonToXml.h"
#include <sstream>

using namespace std;
using namespace rapidjson;

const std::string xmldisplay("<display agent=\"Navi\" content=\"SearchResult\" errcode=\"0\">"\
                            "<count>0</count>"\
                                 "<items>"\
                                        "<item>"\
                                               "<t>poi</t>"\
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
                                                "<typs>song</typs>"\
                                                "<songid>11</songid>"\
                                                "<songname>songname12</songname>"\
                                                "<albumid>albumid13</albumid>"\
                                                "<albumname>albumname14</albumname>"\
                                                "<artistid>artistid15</artistid>"\
                                                "<artistname>artistname16</artistname>"\
                                                "<sourceid>sourceid17</sourceid>"\
                                        "</item>"\
                                        "<item>"\
                                               "<typs>phone</typs>"\
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
                                "<id>11</id>" \
                                "<id>12</id>" \
                                "<e></e>" \
                                "<type attr=\"test\">updatehints</type>" \
                                "<params src=\"demo\">" \
                                    "<scenario>DISPLAY_HINTS_ONLY</scenario>" \
                                    "<hint attr=\"frist\">aa</hint>" \
                                    "<hint>bb</hint>" \
                                    "<hint>cc</hint>" \
                                    "<data>世纪汇广场A座</data>" \
                                    "<longitude>12.45</longitude>" \
                                    "<latitude>142.68</latitude>" \
                                    "<data>世纪汇广场B座</data>" \
                                    "<longitude>67.8</longitude>" \
                                    "<latitude>95.67</latitude>" \
                                    "<items>" \
                                        "<item>" \
                                          "<days>monday</days>" \
                                          "<mouth>09</mouth>" \
                                          "<year>1994</year>" \
                                        "</item>" \
                                    "</items>" \
                                "</params>" \
                                "<hello></hello>" \
                            "</action>");

int childNodeCount(pugi::xml_node node)
{
    int count = 0;
    for (pugi::xml_node chNode = node.first_child(); chNode; chNode = chNode.next_sibling()) {
        count++;
    }

    return count;
}

void transferXmlAttr(pugi::xml_node node, pugi::xml_node& data)
{
    for (pugi::xml_node::attribute_iterator attr = node.attributes_begin(); attr != node.attributes_end(); ++attr) {
        std::string attrName = attr->name();
        std::string attrVal = attr->value();

        if (attrName.compare("") == 0 || attrVal.compare("") == 0) {
            printf("Xml2JsonAddAttr error, node attribute is empty \n");
        }
        else {
            data.append_attribute(attrName.c_str()).set_value(attrVal.c_str());
            printf("attrName = [%s], attrVal = [%s]\n", attrName.c_str(), attrVal.c_str());
        }
    }
}

void transferNodes(pugi::xml_node nodes, pugi::xml_node& data)
{
    pugi::xml_node_type curNodeType = nodes.type();
    pugi::xml_node_type childNodeType = nodes.first_child().type();
    pugi::xml_node tarNode;
    printf("node name = [%s] type = [%d] childType = [%d]\n", nodes.name(), curNodeType, childNodeType);
    switch (curNodeType) {
    case pugi::node_pi:
    case pugi::node_declaration:
    case pugi::node_doctype:
    case pugi::node_comment:
        return;
    case pugi::node_pcdata:
    case pugi::node_cdata:
        if (nodes.value()) {
            data.set_value(nodes.value());
        }
        break;
    case pugi::node_element:
        tarNode = data.append_child(nodes.name());
        transferXmlAttr(nodes, tarNode);
        if (childNodeType == pugi::node_null) { // i.e : <hy/> or <hy></hy>
            tarNode.append_child(pugi::xml_node_type::node_null);
            return;
        }
        else if (childNodeCount(nodes) == 1 && (childNodeType == pugi::node_pcdata)) {
            tarNode.append_child(pugi::xml_node_type::node_pcdata).set_value(nodes.child_value());
            return;
        }
        else if (childNodeType == pugi::node_element) {
            for (pugi::xml_node chNode = nodes.first_child(); chNode; chNode = chNode.next_sibling()) {
                transferNodes(chNode, tarNode);
            }
        }
        break;
    default:
        break;
    }
}

std::string transferXml2Xml(const std::string& xml, const std::string &nodePath)
{
    pugi::xml_document doc;
    if (!doc.load_string(xml.c_str(),  pugi::parse_full)) {
        printf("m_dom.load_string() is fail !");
    }

    pugi::xml_node nodelist = doc.select_node((nodePath).c_str()).node();
    pugi::xml_document result;
    result.load_string("");

    pugi::xml_node rootNode = result.append_child(nodelist.name());
    rootNode.set_value(nodelist.value());

    transferXmlAttr(nodelist, rootNode);
    for (pugi::xml_node _node = nodelist.first_child(); _node; _node = _node.next_sibling()) {
        transferNodes(_node, rootNode);
    }

    std::ostringstream oss;
    result.print(oss);
    return oss.str();
}

int main()
{
    XmlToJson str(xmldisplay);
    std::string json;
    std::string xml;

    str.transferXml2Json(json, "display");

    printf("json = %s\n", json.c_str());


    printf("============================================================\n");
    JsonToXml str1(json);

    str1.transferJson2Xml(xml, "display");
    printf("xml = %s\n", xml.c_str());
}
