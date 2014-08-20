#ifndef _IO_NODE_H
#define _IO_NODE_H

#include "project_param.h"
#include "power_node.h"

enum {
    //半双工
    enum_WORK_TYPE_HALF_DUPLEX  = 0,
    //全双工
    enum_WORK_TYPE_DUPLEX,

};
//io类型定义
enum{
    IO_TYPE_BEGIN       = 0,
    //对上的tcp客户端
    IO_TYPE_EXT_CLIENT  = IO_TYPE_BEGIN,
    //未使用
    IO_TYPE_INNER_CLIENT,
    //未使用
    IO_TYPE_INNER_SERVER,
    //对下的com类型
    IO_TYPE_EXT_COM,
    IO_TYPE_END,
};

static const char *io_type_str_array[IO_TYPE_END] = {
    "ext_client",
    "inner_client",
    "inner_server",
    "down_rs485",
};

//io_node节点类型 个数定义
static const char io_node_no[IO_TYPE_END] = {
     def_IO_TCP_SERVER_NODE_NO,
     def_IO_TCP_CLIENT_NODE_NO,
     def_IO_TCP_EXT_CLIENT_NODE_NO,
     def_IO_COM_EXT_NODE_NO,
};

class io_node{
public:
	io_node():pio_node_map_(NULL){list_init(&device_head_);}
	~io_node(){}

	void name_set(const char *name){strncpy(name_, name, sizeof(name_));}
	const char *name_get(void) const{return name_;}

	void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

	void process_set(const char *process){strncpy(process_, process, sizeof(process_));}
	const char *process_get(void){return process_;}

	void protocol_set(const char *protocol){strncpy(protocol_, protocol, sizeof(protocol_));}
	const char *protocol_get(void){return protocol_;}

	void type_set(const char *type){strncpy(type_, type, sizeof(type_));}
	const char *type_get(void){return type_;}

	void map_set(const char *map){strncpy(map_, map, sizeof(map_));}
	const char *map_get(void){return map_;}

	//此io_node节点所关联的其他io_node结点
	void io_node_map_set(io_node *pnode){pio_node_map_ = pnode;}
	io_node *io_node_map_get(void){return pio_node_map_;}

    int duplextype_get(void) const
    {
        int     type    = io_type_get(type_);

        //IO_TYPE_EXT_COM 为485总线 所以为单双工的
        if (type == IO_TYPE_EXT_COM){
            return enum_WORK_TYPE_HALF_DUPLEX;
        }

        return enum_WORK_TYPE_DUPLEX;
    }

    //由io_node的type值， 得到类型索引
    static int io_type_get(const char *type_str)
    {
        int     i                  = IO_TYPE_BEGIN;

        for (; i < IO_TYPE_END; i++){
            if (0 == strcmp(type_str, io_type_str_array[i])){
                break;
            }
        }

        return i;
    }

    int io_type_get(void) const
    {
        return io_type_;
    }
    bool io_type_set(int type)
    {
        if (type >= IO_TYPE_END){
            return false;
        }
        io_type_    = type;

        return true;
    }

    //此io_node结点下所挂载的设备链表头
    list_head_t * device_list_head_get(void)
    {
        return &device_head_;
    }
private:
	char   			name_[def_NAME_MAX_LEN];
	char   			describe_[def_DESCRIBE_MAX_LEN];
	char   			process_[def_NAME_MAX_LEN];
	char   			protocol_[def_NAME_MAX_LEN];
	char   			type_[def_NAME_MAX_LEN];
	char   			map_[def_NAME_MAX_LEN];//映射资源

	int             io_type_;
	list_head_t     device_head_;                       //此io_node结点下所挂载的设备链表头
	io_node         *pio_node_map_;                     //此io_node所关联的io_node节点指针
};

//tcp_server节点
class io_tcp_server_node:public io_node{
public:
	io_tcp_server_node(){}
	~io_tcp_server_node(){}

	void server_ip_set(const char *server_ip){strncpy(server_ip_, server_ip, sizeof(server_ip_));}
	const char *server_ip_get(void){return server_ip_;}

	void server_port_set(int port){server_port_ = port;}
	int server_port_get(void){return server_port_;}
private:
	char   			server_ip_[def_NAME_MAX_LEN];
	int 			server_port_;
};

//tcp 客户端
class io_tcp_client_node:public io_tcp_server_node{
public:
	io_tcp_client_node(){}
	~io_tcp_client_node(){}

	void client_ip_set(const char *client_ip){strncpy(client_ip_, client_ip, sizeof(client_ip_));}
	const char *client_ip_get(void){return client_ip_;}
private:
	char   			client_ip_[def_NAME_MAX_LEN];
};

//对外通讯tcp cient定义  即对上通讯
class io_tcp_ext_client_node:public io_tcp_client_node{
public:
	io_tcp_ext_client_node(){}
	~io_tcp_ext_client_node(){}
};

//com基类
class io_com_node:public io_node{
public:
	io_com_node(){}
	~io_com_node(){}

	void com_set(const char *com){strncpy(com_, com, sizeof(com_));}
	const char * com_get(void){return com_;}

	void power_group_set(const char *power_group){strncpy(power_group_, power_group, sizeof(power_group_));}
	const char * power_group_get(void){return power_group_;}

	//电源控制部分
	void power_node_set(power_node *power_node){power_node_ = power_node;}
	power_node * power_node_get(void){return power_node_;}

	bool power_ctrl(char value)
	{
	    if (NULL == power_node_){
	        return false;
	    }
	    return power_node_->power_ctrl(value);
	}

    int bps_set(int bps)
    {
        bps_ = bps;
        return 0;
    }
    int bps_get(void)
    {
        return bps_;
    }

    int stop_set(int stop)
    {
        stop_ = stop;
        return 0;
    }
    int stop_get(void)
    {
        return stop_;
    }

    int bits_set(int bits)
    {
        bits_ = bits;
        return 0;
    }
    int bits_get(void)
    {
        return bits_;
    }

    int parity_set(int parity)
    {
        parity_ = parity;
        return 0;
    }
    int parity_get(void)
    {
        return parity_;
    }

	int ios_get(char **com, int &bps, int &stop, int &bits, int &parity)
	{
		*com						= com_;
		bps							= bps_;
		stop 						= stop_;
		bits 						= bits_;
		parity						= parity_;

		return 0;
	}

	//实际应用中  未使用
    int send_interval_set(int send_interval)
    {
        send_interval_ = send_interval;
        return 0;
    }
	//实际应用中  未使用
    int send_retry_cnt_set(int send_retry_cnt)
    {
        send_retry_cnt_ = send_retry_cnt;
        return 0;
    }
	//实际应用中  未使用
    int recv_timeout_set(int recv_timeout)
    {
        recv_timeout_ = recv_timeout;
        return 0;
    }
	//实际应用中  未使用
	int send_recv_param_get(int &send_interval, int &send_retry_cnt, int &recv_timeout)
	{
		send_interval 				= send_interval_;
		send_retry_cnt				= send_retry_cnt_;
		recv_timeout				= recv_timeout_;

		return 0;
	}

private:
	power_node      *power_node_;
	char   			com_[def_NAME_MAX_LEN];
	char   			power_group_[def_NAME_MAX_LEN];
	int 			bps_;
	int 			bits_;
	int 			stop_;
	int 			parity_;
	int 			send_interval_;
	int 			send_retry_cnt_;
	int 			recv_timeout_;
};

#define 	def_DEVICE_ADDR_CHAR_NO 			(4)
#define 	def_SENSOR_ADDR_CHAR_NO 			(4)
#define    C2N(c)                              ((c) - '0')
#define    N2C(n)                              ((n) + '0')
//对下通讯的 com节点
class io_com_ext_node:public io_com_node{
public:
	io_com_ext_node(){}
	~io_com_ext_node(){}

	//设备地址设定
    int device_addr_set(const char *device_addr)
    {
        int     value;

        value   = strtoul(device_addr, NULL, 16);
//        value   = htobe32(value);
        device_addr_    = value;

        return 0;
    }
    uint32 device_addr_get(void)
    {
        return device_addr_;
    }
    int device_addr_no_get(void){return sizeof(device_addr_);}

    //传感器地址设置
    int sensor_addr_set(const char *sensor_addr)
    {
        int     value;

        value   = strtoul(sensor_addr, NULL, 16);
//        value   = htobe32(value);
        sensor_addr_    = value;

        return 0;
    }
    uint32 sensor_addr_get(void)
    {
        return sensor_addr_;
    }
    int sensor_addr_no_get(void){return sizeof(sensor_addr_);}
private:
	uint32 			device_addr_;
	uint32 			sensor_addr_;
};

//io节点配置结构体
class io_config{
public:
    io_config()
    {
        memset(index_, 0, sizeof(index_));
    }
	~io_config(){}

    void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

	//依据io_node的type 将io_node添加到对应的io_node容器中
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
	//依据io_node的type和容器内的索引 获取io_node指针
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

    //用于遍历整个io_type节点
    int io_type_start(void){return IO_TYPE_BEGIN;}
    int io_type_end(void){return IO_TYPE_END;}

    //依据name在io_type容器中查找io_node节点
    io_node *io_node_find(const char *name)
    {
        int         i, j, io_vector_no;
        io_node     *pio_node;

        for (i = io_type_start(); i < io_type_end(); i++){
            io_vector_no                    = io_vector_no_get(i);
            for (j = 0; j < io_vector_no; j++){
                pio_node                    = io_vector_get(i, j);
                //查找io配置中属于当前进程的io_node
                if (0 == strcmp(name, pio_node->name_get())){
                    return pio_node;
                }
            }
        }

        return NULL;
    }
private:
    char   			    describe_[def_DESCRIBE_MAX_LEN];
	unsigned char       index_[IO_TYPE_END];
	io_tcp_server_node  io_tcp_server_node_vector[def_IO_TCP_SERVER_NODE_NO];
	io_tcp_client_node  io_tcp_client_node_vector[def_IO_TCP_CLIENT_NODE_NO];
	io_tcp_ext_client_node  io_tcp_ext_client_node_vector[def_IO_TCP_EXT_CLIENT_NODE_NO];
	io_com_ext_node  io_com_ext_node_vector[def_IO_COM_EXT_NODE_NO];
};



#endif

