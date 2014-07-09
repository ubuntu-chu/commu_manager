#ifndef    _CONFIG_H
#define    _CONFIG_H

//define  xml  config struct
#include <string>
#include <vector>

using std::string;
using std::vector;

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
	process_config(){}
	~process_config(){}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void process_add(const class process_node &node){process_.push_back(node);}
	vector<process_node> &process_vector_get(void){return process_;}
private:
	string 			describe_;
	vector<process_node> process_;
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

	void protocol_add(const class protocol_node &node){protocol_.push_back(node);}
	vector<protocol_node> &protocol_vector_get(void){return protocol_;}
private:
	string 			describe_;
	vector<protocol_node> protocol_;
};

class io_node{
public:
	io_node(){}
	~io_node(){}

	void name_set(const string &name){name_ = name;}
	string &name_get(void){return name_;}

	void describe_set(const string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

	void process_set(const string &process){process_ = process;}
	string &process_get(void){return process_;}

	void protocol_set(const string &protocol){protocol_ = protocol;}
	string &protocol_get(void){return protocol_;}

	void medium_set(const string &medium){medium_ = medium;}
	string &medium_get(void){return medium_;}
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

	void server_port_set(int port){port_ = port;}
	int server_port_get(void){return port;}
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

	void server_port_set(int port){port_ = port;}
	int server_port_get(void){return port;}

	void client_ip_set(const string &ip){client_ip_ = ip;}
	string &client_ip_get(void){return client_ip_;}
private:
	string 			server_ip_;
	string 			server_ip_;
	int 			server_port_;
};




/******************************************************************************
 *                             END  OF  FILE
******************************************************************************/
#endif
