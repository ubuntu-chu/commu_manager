#ifndef    _POWER_NODE_H
#define    _POWER_NODE_H

#include "project_param.h"

//工程配置定义
class power_node{
public:
	power_node(){}
	~power_node(){}

	void name_set(const char *name){strncpy(name_, name, sizeof(name_));}
	const char *name_get(void){return name_;}

	void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

	void path_set(const char *path){strncpy(path_, path, sizeof(path_));}
	const char *path_get(void){return path_;}

	bool power_on(void){return power_ctrl('1');}
	bool power_off(void){return power_ctrl('0');}
	bool power_exist_chk(void)
	{
	    fd_ = open(path_, O_RDWR);
	    return (-1 == fd_)?(false):(true);
	}
	bool power_ctrl(char value)
	{
	    if (-1 == fd_){
	        return false;
	    }
	    if (sizeof(value) == write(fd_, &value, sizeof(value))){
	        return true;
	    }else {
	        return false;
	    }
	}
private:

	char   			name_[def_NAME_MAX_LEN];
	char   			describe_[def_DESCRIBE_MAX_LEN];
	char 			path_[def_FILE_PATH_MAX_LEN];
	int             fd_;
};

class power_config{
public:
    power_config():index_(0){}
	~power_config(){}

    void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

    int power_add(const class power_node &node)
    {
        if (index_ >= def_POWER_NODE_NO){
            return -1;
        }
        power_vecotr_[index_++]   = node;

        return 0;
    }
	power_node *power_vector_get(void){return power_vecotr_;}
	int power_vector_no_get(void){return index_;}

	power_node *power_node_get(const char *name)
	{
	    int  i;
	    power_node *pnode   = &power_vecotr_[0];

	    for (i = 0; i < index_; i++){
	        if (0 == strcmp(name, pnode->name_get())){
	            break;
	        }
	        pnode++;
	    }
	    if (i == index_){
	        return NULL;
	    }

	    return pnode;
	}
private:
	char   			describe_[def_DESCRIBE_MAX_LEN];
	unsigned char index_;
	power_node      power_vecotr_[def_POWER_NODE_NO];
};


/******************************************************************************
 *                             END  OF  FILE
******************************************************************************/
#endif
