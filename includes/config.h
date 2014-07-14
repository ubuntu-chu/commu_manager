#ifndef    _CONFIG_H
#define    _CONFIG_H

#include <io_node.h>
#include <string>
#include <vector>

using std::string;
using std::vector;


//工程配置定义

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
	int process_vector_no_get(void){return index_;}
private:
	char   			describe_[def_DESCRIBE_MAX_LEN];
	unsigned char index_;
	process_node    process_vecotr_[def_PROCESS_NODE_NO];
};


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

class io_config{
public:
    io_config()
    {
        memset(index_, 0, sizeof(index_));
    }
	~io_config(){}

    void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

    int io_add(int type, const class io_node *pnode)
    {
        io_node *pnode_base  = NULL;

        if (type >= IO_TYPE_END){
            return -1;
        }
        if (index_[type] >= io_node_no[type]){
            return -1;
        }
        pnode       =    io_vector_get(type);
        switch (type){
        case IO_TYPE_EXT_CLIENT:

            reinterpret_cast<io_tcp_ext_client_node *>(pnode_base)[index_[type]++] =
                    *reinterpret_cast<io_tcp_ext_client_node *>(const_cast<io_node *>(pnode));
            break;
        case IO_TYPE_INNER_CLIENT:

            reinterpret_cast<io_tcp_client_node *>(pnode_base)[index_[type]++] =
                    *reinterpret_cast<io_tcp_client_node *>(const_cast<io_node *>(pnode));
            break;
        case IO_TYPE_INNER_SERVER:

            reinterpret_cast<io_tcp_server_node *>(pnode_base)[index_[type]++] =
                    *reinterpret_cast<io_tcp_server_node *>(const_cast<io_node *>(pnode));
            break;
        case IO_TYPE_EXT_COM:

            reinterpret_cast<io_com_ext_node *>(pnode_base)[index_[type]++] =
                    *reinterpret_cast<io_com_ext_node *>(const_cast<io_node *>(pnode));
            break;
        }

        return 0;
    }
    io_node *io_vector_get(int type, int index = 0)
    {
        io_node *pnode  = NULL;

        switch (type){
        case IO_TYPE_EXT_CLIENT:

            pnode = reinterpret_cast<io_node *>(&io_tcp_ext_client_node_vector[index]);
            break;
        case IO_TYPE_INNER_CLIENT:

            pnode = reinterpret_cast<io_node *>(&io_tcp_client_node_vector[index]);
            break;
        case IO_TYPE_INNER_SERVER:

            pnode = reinterpret_cast<io_node *>(&io_tcp_server_node_vector[index]);
            break;
        case IO_TYPE_EXT_COM:

            pnode = reinterpret_cast<io_node *>(&io_com_ext_node_vector[index]);
            break;
        }

        return pnode;
    }

    int io_vector_no_get(int type)
    {
        return index_[type];
    }
    int io_vector_no_inc(int type)
    {
        return index_[type]++;
    }


    int io_type_start(void){return IO_TYPE_BEGIN;}
    int io_type_end(void){return IO_TYPE_END;}
private:
    char   			    describe_[def_DESCRIBE_MAX_LEN];
	unsigned char       index_[IO_TYPE_END];
	io_tcp_server_node  io_tcp_server_node_vector[def_IO_TCP_SERVER_NODE_NO];
	io_tcp_client_node  io_tcp_client_node_vector[def_IO_TCP_CLIENT_NODE_NO];
	io_tcp_ext_client_node  io_tcp_ext_client_node_vector[def_IO_TCP_EXT_CLIENT_NODE_NO];
	io_com_ext_node  io_com_ext_node_vector[def_IO_COM_EXT_NODE_NO];
};


class device_node{
public:
	device_node(){}
	~device_node(){}

    void name_set(const char *name){strncpy(name_, name, sizeof(name_));}
	const char *name_get(void){return name_;}

	void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

	void vender_set(const char *vender){strncpy(vender_, vender, sizeof(vender_));}
	const char *vender_get(void){return vender_;}

	void io_set(const char *io){strncpy(io_, io, sizeof(io_));}
	const char *io_get(void){return io_;}

	void id_set(const int &id){id_ = id;}
	int &id_get(void){return id_;}
private:
	char   			name_[def_NAME_MAX_LEN];
	char   			describe_[def_DESCRIBE_MAX_LEN];
	char 			vender_[def_DESCRIBE_MAX_LEN];
	char 			io_[def_NAME_MAX_LEN];
	int             id_;
};

class device_config{
public:
	device_config(){}
	~device_config(){}

    void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

    int device_add(const class device_node &node)
    {
        if (index_ >= def_PROCESS_NODE_NO){
            return -1;
        }
        device_vecotr_[index_++]   = node;

        return 0;
    }
	device_node *device_vector_get(void){return device_vecotr_;}
	int device_vector_no_get(void){return index_;}
private:
    char   			describe_[def_DESCRIBE_MAX_LEN];
	unsigned char   index_;
	device_node     device_vecotr_[def_PROCESS_NODE_NO];
};

class project_config{
public:
	project_config(){}
	~project_config(){}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void process_config_set(process_config &config){process_ 	= config;}
	process_config &process_config_get(void){return process_;}

	void protocol_config_set(protocol_config &config){protocol_ 	= config;}
	protocol_config &protocol_config_get(void){return protocol_;}

	void io_config_set(io_config &config){io_ 	= config;}
	io_config &io_config_get(void){return io_;}

	void device_config_set(device_config &config){device_ 	= config;}
	device_config &device_config_get(void){return device_;}
private:
	string 						describe_;
	process_config 				process_;
	protocol_config 			protocol_;
	io_config 					io_;
	device_config 				device_;
};

/******************************************************************************
 *                             END  OF  FILE
******************************************************************************/
#endif
