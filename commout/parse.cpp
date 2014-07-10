#include "parse.h"
#include <tinyxml/tinyxml.h>
#include <string>
#include <vector>
#include <map>

using std::string;
class shmem		t_shmem;

bool xml_load(TiXmlDocument&doc, const char *path)
{
    //load file
    if (!doc.LoadFile(path))
    {
		LOG_ERROR << "load xml failed!";
        return false;
    }
    //print
    doc.Print();
    return true;
}

bool xml_save(TiXmlDocument&doc, const char *path)
{
    //save file
	if (!doc.SaveFile(path)) {
		LOG_ERROR << "save xml failed!";
        return false;
	}else {
		LOG_ERROR << "save xml success!";
	}
	return true;
}

/*!
*  /brief 通过根节点和节点名获取节点指针。
*
*  /param pRootEle   xml文件的根节点。
*  /param strNodeName  要查询的节点名
*  /param Node      需要查询的节点指针
*  /return 是否找到。true为找到相应节点指针，false表示没有找到相应节点指针。
*/
bool xml_GetNodePointerByName(TiXmlElement* pRootEle, std::string &strNodeName,
        TiXmlElement* &Node)
{
	// 假如等于根节点名，就退出
	if (strNodeName == pRootEle->Value()) {
		Node = pRootEle;
		return true;
	}
	TiXmlElement* pEle = pRootEle;
    for (pEle = pRootEle->FirstChildElement(); pEle;
            pEle = pEle->NextSiblingElement()) {
        //递归处理子节点，获取节点指针
        if (xml_GetNodePointerByName(pEle, strNodeName, Node))
            return true;
    }
	return false;
}

/*!
*  /brief 通过节点查询。
*
*  /param XmlFile   xml文件全路径。
*  /param strNodeName  要查询的节点名
*  /param strText      要查询的节点文本
*  /return 是否成功。true为成功，false表示失败。
*/
bool xml_QueryNode_Text(TiXmlElement *pRootEle, std::string strNodeName,
        std::string &strText)
{
    if (NULL == pRootEle) {
        return false;
    }
    TiXmlElement *pNode = NULL;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL != pNode) {
        strText = pNode->GetText();
        return true;
    } else {
        return false;
    }
}
/*!
*  /brief 通过节点查询。
*
*  /param XmlFile   xml文件全路径。
*  /param strNodeName  要查询的节点名
*  /param AttMap      要查询的属性值，这是一个map，前一个为属性名，后一个为属性值
*  /return 是否成功。true为成功，false表示失败。
*/
bool xml_QueryNode_Attribute(TiXmlElement *pRootEle, std::string strNodeName,
        std::map<std::string, std::string> &AttMap)
{
    // 定义一个TiXmlDocument类指针
    typedef std::pair<std::string, std::string> String_Pair;
    if (NULL == pRootEle) {
        return false;
    }
    TiXmlElement *pNode = NULL;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL != pNode) {
        TiXmlAttribute* pAttr = NULL;
        for (pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
            std::string strAttName = pAttr->Name();
            std::string strAttValue = pAttr->Value();
            AttMap.insert(String_Pair(strAttName, strAttValue));
        }
        return true;
    } else {
        return false;
    }
    return true;
}

bool xml_QueryNode_Attribute(TiXmlElement *pRootEle, std::string strNodeName,
        std::string &attr_name, std::string &value)
{
    // 定义一个TiXmlDocument类指针
    typedef std::pair<std::string, std::string> String_Pair;
    if (NULL == pRootEle) {
        return false;
    }
    TiXmlElement *pNode = NULL;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL != pNode) {
        TiXmlAttribute* pAttr = NULL;
        for (pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
            if (attr_name == pAttr->Name()){
                value               = pAttr->Value();
                break;
            }
        }
        if (pAttr == NULL){
            return false;
        }
        return true;
    }
    return false;
}

bool xml_QueryNode_Attribute(TiXmlElement *pNode, std::string &attr_name,
        std::string &value)
{
    if (NULL != pNode) {
        TiXmlAttribute* pAttr = NULL;
        for (pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
            if (attr_name == pAttr->Name()){
                value               = pAttr->Value();
                break;
            }
        }
        if (pAttr == NULL){
            return false;
        }
        return true;
    }
    return false;
}
/*!
*  /brief 删除指定节点的值。
*
*  /param XmlFile xml文件全路径。
*  /param strNodeName 指定的节点名。
*  /return 是否成功。true为成功，false表示失败。
*/
bool xml_DelNode(TiXmlDocument *pDoc, std::string strNodeName)
{
    TiXmlElement *pRootEle = pDoc->RootElement();

    if (NULL == pRootEle) {
        return false;
    }
    TiXmlElement *pNode = NULL;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    // 假如是根节点
    if (pRootEle == pNode) {
        if (pDoc->RemoveChild(pRootEle)) {
            return true;
        } else
            return false;
    }
    // 假如是其它节点
    if (NULL != pNode) {
        TiXmlNode *pParNode = pNode->Parent();
        if (NULL == pParNode) {
            return false;
        }

        TiXmlElement* pParentEle = pParNode->ToElement();
        if (NULL != pParentEle) {
            if (pParentEle->RemoveChild(pNode))
                return true;
            else
                return false;
        }
    } else {
        return false;
    }
    return false;
}

/*!
*  /brief 修改指定节点的文本。
*
*  /param XmlFile xml文件全路径。
*  /param strNodeName 指定的节点名。
*  /param strText 重新设定的文本的值
*  /return 是否成功。true为成功，false表示失败。
*/
bool xml_ModifyNode_Text(TiXmlElement *pRootEle, std::string strNodeName,
        std::string strText)
{
    if (NULL == pRootEle) {
        return false;
    }
    TiXmlElement *pNode = NULL;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL != pNode) {
        pNode->Clear();  // 首先清除所有文本
        // 然后插入文本，保存文件
        TiXmlText *pValue = new TiXmlText(strText);
        pNode->LinkEndChild(pValue);
        return true;
    } else
        return false;
}
/*!
*  /brief 修改指定节点的属性值。
*
*  /param XmlFile xml文件全路径。
*  /param strNodeName 指定的节点名。
*  /param AttMap 重新设定的属性值，这是一个map，前一个为属性名，后一个为属性值
*  /return 是否成功。true为成功，false表示失败。
*/
bool xml_ModifyNode_Attribute(TiXmlElement *pRootEle, std::string strNodeName,
        std::map<std::string, std::string> &AttMap)
{
    typedef std::pair<std::string, std::string> String_Pair;
    if (NULL == pRootEle) {
        return false;
    }

    TiXmlElement *pNode = NULL;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL != pNode) {
        TiXmlAttribute* pAttr = NULL;
        std::string strAttName;
        for (pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
            strAttName = pAttr->Name();
            std::map<std::string, std::string>::iterator iter;
            for (iter = AttMap.begin(); iter != AttMap.end(); iter++) {
                if (strAttName == iter->first) {
                    pAttr->SetValue(iter->second);
                }
            }
        }
        return true;
    } else {
        return false;
    }
}

/*!
 *  /brief 增加指定节点的文本。
 *
 *  /param XmlFile xml文件全路径。
 *  /param strParNodeName 要增加的节点的父节点。
 *  /param strNodeName 指定的节点名。
 *  /param strText 要增加的文本
 *  /return 是否成功。true为成功，false表示失败。
 */
bool xml_AddNode_Text(TiXmlElement *pRootEle, std::string strParNodeName,
        std::string strNodeName, std::string strText)
{
    if (NULL == pRootEle) {
        return false;
    }
    TiXmlElement *pNode = NULL;
    xml_GetNodePointerByName(pRootEle, strParNodeName, pNode);
    if (NULL != pNode) {
        // 生成子节点：pNewNode
        TiXmlElement *pNewNode = new TiXmlElement(strNodeName);
        if (NULL == pNewNode) {
            return false;
        }
        // 设置节点文本，然后插入节点
        TiXmlText *pNewValue = new TiXmlText(strText);
        pNewNode->LinkEndChild(pNewValue);
        pNode->InsertEndChild(*pNewNode);
        return true;
    } else
        return false;

}
/*!
 *  /brief 增加节点。
 *
 *  /param XmlFile xml文件全路径。
 *  /param strParNodeName 要增加的节点的父节点。
 *  /param strNodeName 指定的节点名。
 *  /param AttMap 要增加的节点设定的属性值，这是一个map，前一个为属性名，后一个为属性值
 *  /return 是否成功。true为成功，false表示失败。
 */
bool xml_AddNode_Attribute(TiXmlElement *pRootEle, std::string strParNodeName,
        std::string strNodeName, std::map<std::string, std::string> &AttMap)
{
    if (NULL == pRootEle) {
        return false;
    }
    TiXmlElement *pNode = NULL;
    xml_GetNodePointerByName(pRootEle, strParNodeName, pNode);
    if (NULL != pNode) {
        // 生成子节点：pNewNode
        TiXmlElement *pNewNode = new TiXmlElement(strNodeName);
        if (NULL == pNewNode) {
            return false;
        }
        // 设置节点的属性值，然后插入节点
        std::map<std::string, std::string>::iterator iter;
        for (iter = AttMap.begin(); iter != AttMap.end(); iter++) {
            pNewNode->SetAttribute(iter->first, iter->second);
        }
        pNode->InsertEndChild(*pNewNode);
        return true;
    } else
        return false;
}


int xml_parse(const char *path)
{
	void	        *pshmem_addr 	    = reinterpret_cast<void *>(t_shmem.attach());
	project_config	t_project_config;
	process_config 	&process_conf	    = t_project_config.process_config_get();
	protocol_config &protocol_conf	    = t_project_config.protocol_config_get();
	io_config       &io_conf	        = t_project_config.io_config_get();
	device_config   &device_conf	    = t_project_config.device_config_get();
	string          strNodeName;
	string          strAttrName;
	string          value;

	TiXmlDocument doc;
	TiXmlElement *pNode = NULL;

	if (!xml_load(doc, path)){
	    return  -1;
	}
	TiXmlElement *pRootEle = doc.RootElement();
	//解析process配置信息
	strNodeName                         = "process";
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL == pNode) {
	    return  -1;
    }
    strAttrName                         ="describe";
    if (false == xml_QueryNode_Attribute(pNode, strAttrName, value)){
	    return  -1;
    }
    process_conf.describe_set(value);



	return 0;
}


