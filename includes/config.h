#ifndef    _CONFIG_H
#define    _CONFIG_H

//define  xml  config struct
#include <string>
#include <vector>

using std::string;
using std::vector;

//进程间共享内存定义
#define 	def_FTOK_PATH 			("/etc/profile")
#define 	def_FTOK_PROJ_ID 		(0x20)
#define 	def_SHMEM_SIZE			(4096)

//工程配置定义

class process_node{
public:
	process_node(){}
	~process_node(){}

	void name_set(const string &name){name_ = name;}
	string &name_get(void){return name_;}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void file_path_set(const string &file_path){file_path_ = file_path;}
	string &file_path_get(void){return file_path_;}
private:
	string 			name_;
	string 			describe_;
	string 			file_path_;
};
class process_config{
public:
    process_config()
    {
        describe_.reserve(50);
        process_vector_.reserve(5);
    }
	~process_config(){}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void process_add(const class process_node &node){process_vector_.push_back(node);}
	vector<process_node> &process_vector_get(void){return process_vector_;}
private:
	string 			describe_;
	vector<process_node> process_vector_;
};


class protocol_node{
public:
	protocol_node(){}
	~protocol_node(){}

	void name_set(const string &name){name_ = name;}
	string &name_get(void){return name_;}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void file_path_set(const string &file_path){file_path_ = file_path;}
	string &file_path_get(void){return file_path_;}
private:
	string 			name_;
	string 			describe_;
	string 			file_path_;
};
class protocol_config{
public:
	protocol_config(){}
	~protocol_config(){}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void protocol_add(const class protocol_node &node){protocol_vector_.push_back(node);}
	vector<protocol_node> &protocol_vector_get(void){return protocol_vector_;}
private:
	string 			describe_;
	vector<protocol_node> protocol_vector_;
};

class io_node{
public:
	io_node(const char *type = NULL):type_(type){}
	~io_node(){}

	void name_set(const string &name){name_ = name;}
	string &name_get(void){return name_;}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void process_set(const string &process){process_ = process;}
	string &process_get(void){return process_;}

	void protocol_set(const string &protocol){protocol_ = protocol;}
	string &protocol_get(void){return protocol_;}

	void type_set(const string &type){type_ = type;}
	string &type_get(void){return type_;}
private:
	string 			name_;
	string 			describe_;
	string 			process_;
	string 			protocol_;
	string 			type_;       //io节点类型
};

class io_tcp_server_node:io_node{
public:
	io_tcp_server_node(){}
	~io_tcp_server_node(){}

	void server_ip_set(const string &ip){server_ip_ = ip;}
	string &server_ip_get(void){return server_ip_;}

	void server_port_set(int port){server_port_ = port;}
	int server_port_get(void){return server_port_;}
private:
	string 			server_ip_;
	int 			server_port_;
};

class io_tcp_client_node:io_node{
public:
	io_tcp_client_node(){}
	~io_tcp_client_node(){}

	void server_ip_set(const string &ip){server_ip_ = ip;}
	string &server_ip_get(void){return server_ip_;}

	void server_port_set(int port){server_port_ = port;}
	int server_port_get(void){return server_port_;}

	void client_ip_set(const string &ip){client_ip_ = ip;}
	string &client_ip_get(void){return client_ip_;}
private:
	string 			server_ip_;
	string 			client_ip_;
	int 			server_port_;
};

//对外通讯tcp cient定义
class io_tcp_ext_client_node:io_tcp_client_node{
public:
	io_tcp_ext_client_node(){}
	~io_tcp_ext_client_node(){}

	void map_set(const string &map){map_ = map;}
	string &map_get(void){return map_;}
private:
	string 			map_;				//此客户端所映射的资源
};


class io_com_node:io_node{
public:
	io_com_node():io_node("rs485"){}
	~io_com_node(){}

	int ios_set(const string &com, int &bps, int &stop, int &parity)
	{
		com_						= com;
		bps_						= bps;
		stop_ 						= stop;
		parity_						= parity;

		return 0;
	}
	int ios_get(string &com, int &bps, int &stop, int &parity)
	{
		com							= com_;
		bps							= bps_;
		stop 						= stop_;
		parity						= parity_;

		return 0;
	}

	int send_recv_param_set(int &send_interval, int &send_retry_cnt, int &recv_timeout)
	{
		send_interval_ 				= send_interval;
		send_retry_cnt_				= send_retry_cnt;
		recv_timeout_				= recv_timeout;

		return 0;
	}
	int send_recv_param_get(int &send_interval, int &send_retry_cnt, int &recv_timeout)
	{
		send_interval 				= send_interval_;
		send_retry_cnt				= send_retry_cnt_;
		recv_timeout				= recv_timeout_;

		return 0;
	}

private:
	string 			com_;
	int 			bps_;
	int 			stop_;
	int 			parity_;
	int 			send_interval_;
	int 			send_retry_cnt_;
	int 			recv_timeout_;
};

#define 	def_DEVICE_ADDR_CHAR_NO 			(4)
#define 	def_SENSOR_ADDR_CHAR_NO 			(4)

class io_com_ext_node:io_com_node{
public:
	io_com_ext_node(){}
	~io_com_ext_node(){}

	void map_set(const string &map){map_ = map;}
	string &map_get(void){return map_;}
private:
	string 			map_;
	char 			device_addr[def_DEVICE_ADDR_CHAR_NO];
	char 			sensor_addr[def_SENSOR_ADDR_CHAR_NO];
};

class io_config{
public:
	io_config(){}
	~io_config(){}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

#if 0
	void io_add(const class io_node &node){io_tcp_ext_client_vector_.push_back(node);}
	vector<io_node> &io_vector_get(void){return io_tcp_ext_client_vector_;}
#endif
private:
	string 			describe_;
	vector<io_tcp_ext_client_node> io_tcp_ext_client_vector_;
	vector<io_tcp_client_node> 		io_tcp_client_vector_;
	vector<io_tcp_server_node> 		io_tcp_server_vector_;
	vector<io_com_ext_node> 		io_com_ext_vector_;
};


class device_node{
public:
	device_node(){}
	~device_node(){}

	void name_set(const string &name){name_ = name;}
	string &name_get(void){return name_;}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void vender_set(const string &vender){vender_ = vender;}
	string &vender_get(void){return vender_;}

	void io_set(const string &io){io_ = io;}
	string &io_get(void){return io_;}

	void id_set(const int &id){id_ = id;}
	int &id_get(void){return id_;}
private:
	string 			name_;
	string 			describe_;
	string 			vender_;
	string 			io_;
	int 			id_;
};
class device_config{
public:
	device_config(){}
	~device_config(){}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void device_add(const class device_node &node){device_vector_.push_back(node);}
	vector<device_node> &device_vector_get(void){return device_vector_;}
private:
	string 			describe_;
	vector<device_node> device_vector_;
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
