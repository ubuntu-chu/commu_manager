#ifndef    _PROTOCOL_NODE_H
#define    _PROTOCOL_NODE_H

#include "project_param.h"

class protocol_node{
public:
	protocol_node(){}
	~protocol_node(){}

	void name_set(const char *name){strncpy(name_, name, sizeof(name_));}
	const char *name_get(void){return name_;}

	void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

	void file_path_set(const char *file_path){strncpy(file_path_, file_path, sizeof(file_path_));}
	const char *file_path_get(void){return file_path_;}
private:
	char   			name_[def_NAME_MAX_LEN];
	char   			describe_[def_DESCRIBE_MAX_LEN];
	char 			file_path_[def_FILE_PATH_MAX_LEN];
};
class protocol_config{
public:
	protocol_config():index_(0){}
	~protocol_config(){}

    void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

    int protocol_add(const class protocol_node &node)
    {
        if (index_ >= def_PROCESS_NODE_NO){
            return -1;
        }
        protocol_vecotr_[index_++]   = node;

        return 0;
    }
	protocol_node *protocol_vector_get(void){return protocol_vecotr_;}
	int protocol_vector_no_get(void){return index_;}

private:
    char   			 describe_[def_DESCRIBE_MAX_LEN];
	unsigned char   index_;
	protocol_node    protocol_vecotr_[def_PROCESS_NODE_NO];
};

/******************************************************************************
 *                             END  OF  FILE
******************************************************************************/
#endif
