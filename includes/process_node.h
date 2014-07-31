#ifndef    _PROCESS_NODE_H
#define    _PROCESS_NODE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class process_node{
public:
	process_node(){}
	~process_node(){}

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
class process_config{
public:
    process_config():index_(0){}
	~process_config(){}

    void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

    int process_add(const class process_node &node)
    {
        if (index_ >= def_PROCESS_NODE_NO){
            return -1;
        }
        process_vecotr_[index_++]   = node;

        return 0;
    }
	process_node *process_vector_get(void){return process_vecotr_;}
	process_node *process_node_get(int index){return &process_vecotr_[index];}
	int process_vector_no_get(void){return index_;}
private:
	char   			describe_[def_DESCRIBE_MAX_LEN];
	unsigned char index_;
	process_node    process_vecotr_[def_PROCESS_NODE_NO];
};



/******************************************************************************
 *                             END  OF  FILE
******************************************************************************/
#endif
