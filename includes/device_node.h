#ifndef _DEVICE_NODE_H
#define _DEVICE_NODE_H

#include "project_param.h"

//io类型定义
enum{
    DEVICE_TYPE_BEGIN       = 0,
    DEVICE_TYPE_RFID_READER  = IO_TYPE_BEGIN,
    DEVICE_TYPE_END,
};

static const char *device_type_str_array[IO_TYPE_END] = {
    "rfid_reader",
};

static const char device_node_no[DEVICE_TYPE_END] = {
     def_DEVICE_RFID_READER_NODE_NO,
};

class device_node{
public:
	device_node(){list_init(&node_);}
	~device_node(){}

    void name_set(const char *name){strncpy(name_, name, sizeof(name_));}
	const char *name_get(void){return name_;}

	void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

	void vender_set(const char *vender){strncpy(vender_, vender, sizeof(vender_));}
	const char *vender_get(void){return vender_;}

	void type_set(const char *type){strncpy(type_, type, sizeof(type_));}
	const char *type_get(void){return type_;}

	void io_set(const char *io){strncpy(io_, io, sizeof(io_));}
	const char *io_get(void){return io_;}

	void id_set(const int &id){id_ = id;}
	int &id_get(void){return id_;}

	void class_type_set(const int &class_type){class_type_ = class_type;}
	int &class_type_get(void){return class_type_;}

    static int device_type_get(const char *type_str)
    {
        int     i                  = DEVICE_TYPE_BEGIN;

        for (; i < DEVICE_TYPE_END; i++){
            if (0 == strcmp(type_str, device_type_str_array[i])){
                break;
            }
        }

        return i;
    }

    list_node_t *node_get(void)
    {
        return &node_;
    }
    static int node_offset_get(void)
    {
        return OFFSET(class device_node, node_);
    }
    static device_node *device_entry(list_node_t *pnode)
    {
        device_node             *pdevice_node;

        if (NULL == pnode){
            return NULL;
        }
        pdevice_node    = list_entry_offset(pnode, class device_node, device_node::node_offset_get());

        return pdevice_node;
    }
private:
	char   			name_[def_NAME_MAX_LEN];
	char   			describe_[def_DESCRIBE_MAX_LEN];
	char 			vender_[def_DESCRIBE_MAX_LEN];
	char   			type_[def_NAME_MAX_LEN];
	char 			io_[def_NAME_MAX_LEN];
	int             id_;
	int             class_type_;

	list_node_t     node_;
};

class device_rfid_reader_node:public device_node{
public:
	device_rfid_reader_node(){}
	~device_rfid_reader_node(){}

	void min_power_set(const int &power){min_power_= power;}
	int &min_power_get(void){return min_power_;}

	void max_power_set(const int &power){max_power_= power;}
	int &max_power_get(void){return max_power_;}

	void min_scantime_set(const int &scantime){min_scantime_= scantime;}
	int &min_scantime_get(void){return min_scantime_;}

	void max_scantime_set(const int &scantime){max_scantime_= scantime;}
	int &max_scantime_get(void){return max_scantime_;}

	void epc_len_set(const int &len){epc_len_ = len;}
	int &epc_len_get(void){return epc_len_;}

	void data_len_set(const int &len){data_len_ = len;}
	int &data_len_get(void){return data_len_;}
private:
	int             min_power_;
	int             max_power_;
	int             min_scantime_;
	int             max_scantime_;
	int             epc_len_;
	int             data_len_;
};


class device_config{
public:
	device_config()
    {
        memset(index_, 0, sizeof(index_));
    }
	~device_config(){}

    void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

    int device_add(int type, const class device_node *pnode)
    {
        device_node *pnode_base  = NULL;

        if (type >= DEVICE_TYPE_END){
            return -1;
        }
        if (index_[type] >= device_node_no[type]){
            return -1;
        }
        pnode       =    device_vector_get(type);
        switch (type){
        case DEVICE_TYPE_RFID_READER:

            reinterpret_cast<device_rfid_reader_node *>(pnode_base)[index_[type]++] =
                    *reinterpret_cast<device_rfid_reader_node *>(const_cast<device_node *>(pnode));
            break;
        }

        return 0;
    }
    device_node *device_vector_get(int type, int index = 0)
    {
        device_node *pnode  = NULL;

        switch (type){
        case DEVICE_TYPE_RFID_READER:

            pnode = reinterpret_cast<device_node *>(&device_rfid_reader_vecotr_[index]);
            break;
        }

        return pnode;
    }

    int device_vector_no_get(int type)
    {
        return index_[type];
    }
    int device_vector_no_inc(int type)
    {
        return index_[type]++;
    }


    int device_type_start(void){return DEVICE_TYPE_BEGIN;}
    int device_type_end(void){return DEVICE_TYPE_END;}
private:
    char   			describe_[def_DESCRIBE_MAX_LEN];
	unsigned char       index_[DEVICE_TYPE_END];
	device_rfid_reader_node     device_rfid_reader_vecotr_[def_DEVICE_RFID_READER_NODE_NO];
};
#endif

