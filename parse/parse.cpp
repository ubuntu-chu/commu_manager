#include "parse.h"
#include "datum.h"

using std::string;

bool xml_load(TiXmlDocument&doc, const char *path)
{
    //load file
    if (!doc.LoadFile(path))
    {
		LOG_ERROR << "load xml failed!";
        return false;
    }
    //print
//    doc.Print();
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

int config_relative_set(void)
{
        //将工程配置信息写入到共享内存后 重新更新引用的值
	project_config	*pproject_config     = t_project_datum.pproject_config_;
	power_config 	&power_conf	        = pproject_config->power_config_get();
//	process_config 	&process_conf	    = pproject_config->process_config_get();
//	protocol_config &protocol_conf	    = pproject_config->protocol_config_get();
	io_config       &io_conf	        = pproject_config->io_config_get();
	device_config   &device_conf	    = pproject_config->device_config_get();
    //建立起通道与设备之间的归属关系
    //将设备挂在到通道的设备链表上   匹配依据是设备的io与通道的name相等
	int             i, j;
	int             ii, jj;
	int             io_vector_no, device_vector_no;
    device_node     *pdevice_node;
    io_node         *pio_node;
    list_head_t     *phead;
    list_node_t     *pnode;

    //初始化io_node上设备链表
    for (i = io_conf.io_type_start(); i < io_conf.io_type_end(); i++){
        io_vector_no                    = io_conf.io_vector_no_get(i);
        for (j = 0; j < io_vector_no; j++){
            pio_node                    = io_conf.io_vector_get(i, j);
            phead                       = pio_node->device_list_head_get();
            list_init(phead);
            //初始化通道电源组
            if (i == IO_TYPE_EXT_COM){
                //与power_node建立联系
                reinterpret_cast<io_com_node *>(pio_node)
                        ->power_node_set(power_conf.power_node_get(
                                reinterpret_cast<io_com_node *>(pio_node)
                                ->power_group_get()));
            }
        }
    }
    //将设备挂接到所属io_node的设备链表上
    for (ii = device_conf.device_type_start(); ii < device_conf.device_type_end(); ii++){
        device_vector_no               = device_conf.device_vector_no_get(ii);
        for (jj = 0; jj < device_vector_no; jj++){
            pdevice_node               = device_conf.device_vector_get(ii, jj);
            pnode                      = pdevice_node->node_get();
            list_init(pnode);
            for (i = io_conf.io_type_start(); i < io_conf.io_type_end(); i++){
                io_vector_no                    = io_conf.io_vector_no_get(i);
                for (j = 0; j < io_vector_no; j++){
                    pio_node                    = io_conf.io_vector_get(i, j);
                    //将设备挂接到所属io_node的设备链表上
                    if (0 == strcmp(pdevice_node->io_get(), pio_node->name_get())){
                        phead                   = pio_node->device_list_head_get();
                        list_insert_after(phead, pnode);
                    }
                }
            }
        }
    }

#if 0
    device_node *pdevice_node;
    list_node_t *pos;
    //获取此通道下io下所挂接设备
    //同一个io下所有设备的class type必须相同
    list_head_t * plist_head;

    //检查每个io_node下所挂载设备的正确性
    for (i = io_conf.io_type_start(); i < io_conf.io_type_end(); i++){
        io_vector_no                    = io_conf.io_vector_no_get(i);
        for (j = 0; j < io_vector_no; j++){
            pio_node                    = io_conf.io_vector_get(i, j);
            plist_head                  = pio_node->device_list_head_get();
            //io_node下设备链表为空  则不进行检查
            if (list_empty(plist_head)){
                continue;
            }
            list_for_each(pos, plist_head){
                pdevice_node    = list_entry_offset(pos, class device_node, device_node::node_offset_get());
            }
        }
    }
#endif

    io_node         *pio_node_map;
    //处理io_node之间的map关系
    for (i = io_conf.io_type_start(); i < io_conf.io_type_end(); i++){
        io_vector_no                    = io_conf.io_vector_no_get(i);
        for (j = 0; j < io_vector_no; j++){
            pio_node                    = io_conf.io_vector_get(i, j);
            if (NULL != (pio_node_map = io_conf.io_node_find(pio_node->map_get()))){
                pio_node->io_node_map_set(pio_node_map);
            }
        }
    }


    return 0;
}

int xml_parse(const char *path)
{
#if 1
	void	        *pshmem_addr 	    = reinterpret_cast<void *>(t_project_datum.shmem_.attach());
	project_config	t_project_config;
	power_config 	&power_conf	        = t_project_config.power_config_get();
	process_config 	&process_conf	    = t_project_config.process_config_get();
	protocol_config &protocol_conf	    = t_project_config.protocol_config_get();
	io_config       &io_conf	        = t_project_config.io_config_get();
	device_config   &device_conf	    = t_project_config.device_config_get();
#else
	project_config  *pproject_config    = reinterpret_cast<project_config *>(t_shmem.attach());
	process_config 	&process_conf	    = pproject_config->process_config_get();
	protocol_config &protocol_conf	    = pproject_config->protocol_config_get();
	io_config       &io_conf	        = pproject_config->io_config_get();
	device_config   &device_conf	    = pproject_config->device_config_get();
#endif
	std::string          strNodeName;
	std::string          strAttrName;
	std::string          value;

	TiXmlDocument doc;
	TiXmlElement *pNode                 = NULL;
	TiXmlElement *pEle                  = NULL;

	if (!xml_load(doc, path)){
	    return  -1;
	}
	TiXmlElement *pRootEle = doc.RootElement();

	//查找日志等级配置
    strAttrName                         = def_LOG_LEV_STRING;
    if (false == xml_QueryNode_Attribute(pRootEle, strAttrName, value)){
	    return  -1;
    }
    if (0 == strcmp(def_LOG_LEV_TRACE, value.c_str())){
        t_project_config.log_lev_set(Logger::TRACE);
    }else if (0 == strcmp(def_LOG_LEV_DEBUG , value.c_str())){
        t_project_config.log_lev_set(Logger::DEBUG);
    }else if (0 == strcmp(def_LOG_LEV_INFO , value.c_str())){
        t_project_config.log_lev_set(Logger::INFO);
    }else if (0 == strcmp(def_LOG_LEV_WARN , value.c_str())){
        t_project_config.log_lev_set(Logger::WARN);
    }else if (0 == strcmp(def_LOG_LEV_ERROR , value.c_str())){
        t_project_config.log_lev_set(Logger::ERROR);
    }else if (0 == strcmp(def_LOG_LEV_FATAL , value.c_str())){
        t_project_config.log_lev_set(Logger::FATAL);
    }

	//解析power配置信息
	strNodeName                         = def_POWER_STRING;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL == pNode) {
	    return  -1;
    }
    strAttrName                         = def_DESCRIBE_STRING;
    if (false == xml_QueryNode_Attribute(pNode, strAttrName, value)){
	    return  -1;
    }
    power_conf.describe_set(value.c_str());
    //遍历pNode下所有节点
    for (pEle = pNode->FirstChildElement(); pEle;
            pEle = pEle->NextSiblingElement()) {
        //遍历此节点下所有属性 并加入到process配置中
        TiXmlAttribute* pAttr = NULL;
        power_node        t_power_node;
        for (pAttr = pEle->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
            if (0 == strcmp(def_NAME_STRING, pAttr->Name())){
                t_power_node.name_set(pAttr->Value());
            }else if (0 == strcmp(def_DESCRIBE_STRING , pAttr->Name())){
                t_power_node.describe_set(pAttr->Value());
            }else if (0 == strcmp(def_PATH_STRING , pAttr->Name())){
                t_power_node.path_set(pAttr->Value());
            }
        }
        //检测power控制文件是否存在
        t_power_node.power_exist_chk();
        //默认power关闭
        t_power_node.power_off();
        power_conf.power_add(t_power_node);
    }

	//解析process配置信息
	strNodeName                         = def_PROCESS_STRING;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL == pNode) {
	    return  -1;
    }
    strAttrName                         = def_DESCRIBE_STRING;
    if (false == xml_QueryNode_Attribute(pNode, strAttrName, value)){
	    return  -1;
    }
    process_conf.describe_set(value.c_str());
    //遍历pNode下所有节点
    for (pEle = pNode->FirstChildElement(); pEle;
            pEle = pEle->NextSiblingElement()) {
        //遍历此节点下所有属性 并加入到process配置中
        TiXmlAttribute* pAttr = NULL;
        process_node        t_process_node;
        for (pAttr = pEle->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
            if (0 == strcmp(def_NAME_STRING, pAttr->Name())){
                t_process_node.name_set(pAttr->Value());
            }else if (0 == strcmp(def_DESCRIBE_STRING , pAttr->Name())){
                t_process_node.describe_set(pAttr->Value());
            }else if (0 == strcmp(def_PATH_STRING , pAttr->Name())){
                t_process_node.file_path_set(pAttr->Value());
            }
        }
        process_conf.process_add(t_process_node);
    }

	//解析protocol配置信息
	strNodeName                         = def_PROTOCOL_STRING;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL == pNode) {
	    return  -1;
    }
    strAttrName                         = def_DESCRIBE_STRING;
    if (false == xml_QueryNode_Attribute(pNode, strAttrName, value)){
	    return  -1;
    }
    protocol_conf.describe_set(value.c_str());
    //遍历pNode下所有节点
    for (pEle = pNode->FirstChildElement(); pEle;
            pEle = pEle->NextSiblingElement()) {
        //遍历此节点下所有属性 并加入到protocol配置中
        TiXmlAttribute* pAttr = NULL;
        protocol_node        t_protocol_node;
        for (pAttr = pEle->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
            if (0 == strcmp(def_NAME_STRING, pAttr->Name())){
                t_protocol_node.name_set(pAttr->Value());
            }else if (0 == strcmp(def_DESCRIBE_STRING , pAttr->Name())){
                t_protocol_node.describe_set(pAttr->Value());
            }else if (0 == strcmp(def_PATH_STRING , pAttr->Name())){
                t_protocol_node.file_path_set(pAttr->Value());
            }
        }
        protocol_conf.protocol_add(t_protocol_node);
    }

	//解析io配置信息
	strNodeName                         = def_IO_STRING;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL == pNode) {
	    return  -1;
    }
    strAttrName                         = def_DESCRIBE_STRING;
    if (false == xml_QueryNode_Attribute(pNode, strAttrName, value)){
	    return  -1;
    }
    io_conf.describe_set(value.c_str());
    //遍历pNode下所有节点
    for (pEle = pNode->FirstChildElement(); pEle;
            pEle = pEle->NextSiblingElement()) {
        //遍历此节点下所有属性 并加入到io配置中
        TiXmlAttribute* pAttr = NULL;
        io_node                 *pio_node;
        int            type;
        strAttrName                         = def_TYPE_STRING;
        if (false == xml_QueryNode_Attribute(pEle, strAttrName, value)){
            return  -1;
        }
        type                        = io_node::io_type_get(value.c_str());
        pio_node                    = io_conf.io_vector_get(type, io_conf.io_vector_no_get(type));
        pio_node->io_type_set(type);
        for (pAttr = pEle->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
            if (0 == strcmp(def_NAME_STRING, pAttr->Name())){
                pio_node->name_set(pAttr->Value());
            }else if (0 == strcmp(def_DESCRIBE_STRING , pAttr->Name())){
                pio_node->describe_set(pAttr->Value());
            }else if (0 == strcmp(def_PROCESS_STRING , pAttr->Name())){
                pio_node->process_set(pAttr->Value());
            }else if (0 == strcmp(def_PROTOCOL_STRING , pAttr->Name())){
                pio_node->protocol_set(pAttr->Value());
            }else if (0 == strcmp(def_TYPE_STRING , pAttr->Name())){
                pio_node->type_set(pAttr->Value());
            }else if (0 == strcmp(def_SERVER_IP_STRING , pAttr->Name())){
                reinterpret_cast<io_tcp_server_node *>(pio_node)
                        ->server_ip_set(pAttr->Value());
            }else if (0 == strcmp(def_SERVER_PORT_STRING , pAttr->Name())){
                reinterpret_cast<io_tcp_server_node *>(pio_node)
                        ->server_port_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_LOCAL_IP_STRING , pAttr->Name())){
                reinterpret_cast<io_tcp_client_node *>(pio_node)
                        ->client_ip_set(pAttr->Value());
            }else if (0 == strcmp(def_MAP_STRING , pAttr->Name())){
                pio_node->map_set(pAttr->Value());

            }else if (0 == strcmp(def_COM_STRING , pAttr->Name())){
                reinterpret_cast<io_com_node *>(pio_node)
                        ->com_set(pAttr->Value());
            }else if (0 == strcmp(def_BPS_STRING , pAttr->Name())){
                reinterpret_cast<io_com_node *>(pio_node)
                        ->bps_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_BITS_STRING , pAttr->Name())){
                reinterpret_cast<io_com_node *>(pio_node)
                        ->bits_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_STOP_STRING , pAttr->Name())){
                reinterpret_cast<io_com_node *>(pio_node)
                        ->stop_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_PARITY_STRING , pAttr->Name())){
                int parity;
                if (0 == strcmp(def_PARITY_NONE_STRING , pAttr->Value())){
                    parity                          = 'n';
                }else if (0 == strcmp(def_PARITY_ODD_STRING , pAttr->Value())){
                    parity                          = 'o';
                }else if (0 == strcmp(def_PARITY_EVEN_STRING , pAttr->Value())){
                    parity                          = 'e';
                }
                reinterpret_cast<io_com_node *>(pio_node)
                        ->parity_set(parity);
            }else if (0 == strcmp(def_SEND_INTERVAL_STRING , pAttr->Name())){
                reinterpret_cast<io_com_node *>(pio_node)
                        ->send_interval_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_SEND_RETRY_CNT_STRING , pAttr->Name())){
                reinterpret_cast<io_com_node *>(pio_node)
                        ->send_retry_cnt_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_RECV_TIMEOUT_STRING , pAttr->Name())){
                reinterpret_cast<io_com_node *>(pio_node)
                        ->recv_timeout_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_DEVICE_ADDR_STRING , pAttr->Name())){
                reinterpret_cast<io_com_ext_node *>(pio_node)
                        ->device_addr_set(pAttr->Value());
            }else if (0 == strcmp(def_SENSOR_ADDR_STRING , pAttr->Name())){
                reinterpret_cast<io_com_ext_node *>(pio_node)
                        ->sensor_addr_set(pAttr->Value());
            }else if (0 == strcmp(def_POWER_GROUP_STRING , pAttr->Name())){
                reinterpret_cast<io_com_node *>(pio_node)
                        ->power_group_set(pAttr->Value());
            }
        }
        io_conf.io_vector_no_inc(type);
    }

	//解析device配置信息
	strNodeName                         = def_DEVICE_STRING;
    xml_GetNodePointerByName(pRootEle, strNodeName, pNode);
    if (NULL == pNode) {
	    return  -1;
    }
    strAttrName                         = def_DESCRIBE_STRING;
    if (false == xml_QueryNode_Attribute(pNode, strAttrName, value)){
	    return  -1;
    }
    device_conf.describe_set(value.c_str());
    //遍历pNode下所有节点
    for (pEle = pNode->FirstChildElement(); pEle;
            pEle = pEle->NextSiblingElement()) {
        //遍历此节点下所有属性 并加入到device配置中
        TiXmlAttribute* pAttr = NULL;
        device_node                 *pnode;
        int            type;
        strAttrName                         = def_TYPE_STRING;
        if (false == xml_QueryNode_Attribute(pEle, strAttrName, value)){
            return  -1;
        }
        type                        = device_node::device_type_get(value.c_str());
        pnode                       = device_conf.device_vector_get(type, device_conf.device_vector_no_get(type));
        for (pAttr = pEle->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
            if (0 == strcmp(def_NAME_STRING, pAttr->Name())){
                pnode->name_set(pAttr->Value());
            }else if (0 == strcmp(def_DESCRIBE_STRING , pAttr->Name())){
                pnode->describe_set(pAttr->Value());
            }else if (0 == strcmp(def_PATH_STRING , pAttr->Name())){
                pnode->vender_set(pAttr->Value());
            }else if (0 == strcmp(def_ID_STRING , pAttr->Name())){
                pnode->id_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_CLASS_TYPE_STRING , pAttr->Name())){
                pnode->class_type_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_IO_STRING , pAttr->Name())){
                pnode->io_set(pAttr->Value());
            }else if (0 == strcmp(def_TYPE_STRING , pAttr->Name())){
                pnode->type_set(pAttr->Value());
            }else if (0 == strcmp(def_MIN_POWER_STRING , pAttr->Name())){
                reinterpret_cast<device_rfid_reader_node *>(pnode)
                        ->min_power_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_MAX_POWER_STRING , pAttr->Name())){
                reinterpret_cast<device_rfid_reader_node *>(pnode)
                        ->max_power_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_MIN_SCANTIME_STRING , pAttr->Name())){
                reinterpret_cast<device_rfid_reader_node *>(pnode)
                        ->min_scantime_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_MAX_SCANTIME_STRING , pAttr->Name())){
                reinterpret_cast<device_rfid_reader_node *>(pnode)
                        ->max_scantime_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_EPC_LEN_STRING , pAttr->Name())){
                reinterpret_cast<device_rfid_reader_node *>(pnode)
                        ->epc_len_set(atoi(pAttr->Value()));
            }else if (0 == strcmp(def_DATA_LEN_STRING , pAttr->Name())){
                reinterpret_cast<device_rfid_reader_node *>(pnode)
                        ->data_len_set(atoi(pAttr->Value()));
            }
        }
        device_conf.device_vector_no_inc(type);
    }

    //将工程配置信息写入到共享内存中
    LOG_INFO << "project_config size = " << sizeof(t_project_config);
	//*reinterpret_cast<project_config *>(pshmem_addr) 	    = t_project_config;
    memcpy(pshmem_addr, &t_project_config, sizeof(t_project_config));
    t_project_datum.pproject_config_        = reinterpret_cast<project_config *>(pshmem_addr);

    config_relative_set();

	return 0;
}


