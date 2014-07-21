#ifndef _IO_NODE_H
#define _IO_NODE_H

#include <includes/includes.h>

//define  xml  config struct
#include <string.h>
#include <endian.h>
#include <stdlib.h>


#define    def_NAME_STRING         ("name")
#define    def_DESCRIBE_STRING     ("describe")
#define    def_FILE_PATH_STRING    ("file_path")
#define    def_VENDER_STRING       ("vender")
#define    def_ID_STRING           ("id")

#define    def_PROCESS_STRING      ("process")
#define    def_PROTOCOL_STRING     ("protocol")
#define    def_IO_STRING           ("io")
#define    def_DEVICE_STRING       ("device")

#define    def_TYPE_STRING         ("type")
#define    def_SERVER_IP_STRING    ("server_ip")
#define    def_SERVER_PORT_STRING  ("server_port")
#define    def_LOCAL_IP_STRING     ("local_ip")
#define    def_MAP_STRING          ("map")

#define    def_COM_STRING          ("com")
#define    def_BITS_STRING         ("bits")
#define    def_BPS_STRING          ("bps")
#define    def_BITS_STRING         ("bits")
#define    def_STOP_STRING         ("stop")
#define    def_PARITY_STRING       ("parity")
#define    def_SEND_INTERVAL_STRING    ("send_interval")
#define    def_SEND_RETRY_CNT_STRING   ("send_retry_cnt")
#define    def_RECV_TIMEOUT_STRING     ("recv_timeout")
#define    def_DEVICE_ADDR_STRING      ("device_addr")
#define    def_SENSOR_ADDR_STRING      ("sensor_addr")

#define    def_NAME_MAX_LEN        (30)
#define    def_DESCRIBE_MAX_LEN    (70)
#define    def_FILE_PATH_MAX_LEN   (30)

#define    def_PROCESS_NODE_NO     (5)
#define    def_PROTOCOL_NODE_NO    (5)
#define    def_IO_TCP_SERVER_NODE_NO          (5)
#define    def_IO_TCP_CLIENT_NODE_NO          (5)
#define    def_IO_TCP_EXT_CLIENT_NODE_NO          (10)
#define    def_IO_COM_EXT_NODE_NO          (10)
#define    def_DEVICE_NODE_NO      (20)

//io类型定义
enum{
    IO_TYPE_BEGIN       = 0,
    IO_TYPE_EXT_CLIENT  = IO_TYPE_BEGIN,
    IO_TYPE_INNER_CLIENT,
    IO_TYPE_INNER_SERVER,
    IO_TYPE_EXT_COM,
    IO_TYPE_END,
};

static const char *io_type_str_array[IO_TYPE_END] = {
    "ext_client",
    "inner_client",
    "inner_server",
    "down_rs485",
};

static const char io_node_no[IO_TYPE_END] = {
     def_IO_TCP_SERVER_NODE_NO,
     def_IO_TCP_CLIENT_NODE_NO,
     def_IO_TCP_EXT_CLIENT_NODE_NO,
     def_IO_COM_EXT_NODE_NO,
};

#define    def_IO_EXT_CLIENT       ("ext_client")
#define    def_IO_INNER_CLIENT     ("inner_client")
#define    def_IO_INNER_SERVER     ("inner_server")
#define    def_IO_EXT_COM          ("down_rs485")


class io_node{
public:
	io_node(){}
	~io_node(){}

	void name_set(const char *name){strncpy(name_, name, sizeof(name_));}
	const char *name_get(void){return name_;}

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
private:
	char   			name_[def_NAME_MAX_LEN];
	char   			describe_[def_DESCRIBE_MAX_LEN];
	char   			process_[def_NAME_MAX_LEN];
	char   			protocol_[def_NAME_MAX_LEN];
	char   			type_[def_NAME_MAX_LEN];
	char   			map_[def_NAME_MAX_LEN];//映射资源
};

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

class io_tcp_client_node:public io_tcp_server_node{
public:
	io_tcp_client_node(){}
	~io_tcp_client_node(){}

	void client_ip_set(const char *client_ip){strncpy(client_ip_, client_ip, sizeof(client_ip_));}
	const char *client_ip_get(void){return client_ip_;}
private:
	char   			client_ip_[def_NAME_MAX_LEN];
};

//对外通讯tcp cient定义
class io_tcp_ext_client_node:public io_tcp_client_node{
public:
	io_tcp_ext_client_node(){}
	~io_tcp_ext_client_node(){}
};


class io_com_node:public io_node{
public:
	io_com_node(){}
	~io_com_node(){}

	void com_set(const char *com){strncpy(com_, com, sizeof(com_));}
	const char * com_get(void){return com_;}

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

    int send_interval_set(int send_interval)
    {
        send_interval_ = send_interval;
        return 0;
    }
    int send_retry_cnt_set(int send_retry_cnt)
    {
        send_retry_cnt_ = send_retry_cnt;
        return 0;
    }
    int recv_timeout_set(int recv_timeout)
    {
        recv_timeout_ = recv_timeout;
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
	char   			com_[def_NAME_MAX_LEN];
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

class io_com_ext_node:public io_com_node{
public:
	io_com_ext_node(){}
	~io_com_ext_node(){}

    int device_addr_set(const char *device_addr)
    {
        int     value;

        value   = strtoul(device_addr, NULL, 16);
        value   = htobe32(value);
        memcpy(device_addr_, &value, sizeof(device_addr_));

        return 0;
    }
    const char *device_addr_get(void)
    {
        return device_addr_;
    }
    int device_addr_no_get(void){return sizeof(device_addr_);}

    int sensor_addr_set(const char *sensor_addr)
    {
        int     value;

        value   = strtoul(sensor_addr, NULL, 16);
        value   = htobe32(value);
        memcpy(sensor_addr_, &value, sizeof(sensor_addr_));

        return 0;
    }
    const char *sensor_addr_get(void)
    {
        return sensor_addr_;
    }
    int sensor_addr_no_get(void){return sizeof(sensor_addr_);}
private:
	char 			device_addr_[def_DEVICE_ADDR_CHAR_NO];
	char 			sensor_addr_[def_SENSOR_ADDR_CHAR_NO];
};

#endif

