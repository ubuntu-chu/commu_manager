#ifndef _PROJECT_PARAM_H
#define _PROJECT_PARAM_H

#include "includes.h"

#define    def_NAME_STRING         ("name")
#define    def_DESCRIBE_STRING     ("describe")
#define    def_PATH_STRING         ("path")
#define    def_VENDER_STRING       ("vender")
#define    def_ID_STRING           ("id")
#define    def_CLASS_TYPE_STRING   ("class")

#define    def_POWER_STRING        ("power")
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
#define    def_POWER_GROUP_STRING  ("power_group")
#define    def_BITS_STRING         ("bits")
#define    def_BPS_STRING          ("bps")
#define    def_BITS_STRING         ("bits")
#define    def_STOP_STRING         ("stop")
#define    def_PARITY_STRING       ("parity")
#define    def_PARITY_NONE_STRING      ("none")
#define    def_PARITY_EVEN_STRING      ("even")
#define    def_PARITY_ODD_STRING       ("odd")
#define    def_SEND_INTERVAL_STRING    ("send_interval")
#define    def_SEND_RETRY_CNT_STRING   ("send_retry_cnt")
#define    def_RECV_TIMEOUT_STRING     ("recv_timeout")
#define    def_DEVICE_ADDR_STRING      ("device_addr")
#define    def_SENSOR_ADDR_STRING      ("sensor_addr")

#define    def_MIN_POWER_STRING    ("min_power")
#define    def_MAX_POWER_STRING    ("max_power")
#define    def_MIN_SCANTIME_STRING ("min_scantime")
#define    def_MAX_SCANTIME_STRING ("max_scantime")
#define    def_EPC_LEN_STRING      ("epc_len")
#define    def_DATA_LEN_STRING     ("data_len")


#define    def_LED_STRING           ("led")
#define    def_LED_RUN_STRING       ("run")
#define    def_LED_ALARM_STRING     ("alarm")

#define    def_EXIST_STRING         ("exist")
#define    def_EXIST_TRUE_STRING     ("true")
#define    def_EXIST_FALSE_STRING     ("false")

#define    def_NAME_MAX_LEN        (30)
#define    def_DESCRIBE_MAX_LEN    (50)
#define    def_FILE_PATH_MAX_LEN   (30)

#define    def_LONG_FILE_PATH_MAX_LEN   (70)
#define    def_EXIST_MAX_LEN    (10)

#define    def_LED_NODE_NO     (3)
#define    def_POWER_NODE_NO     (6)
#define    def_PROCESS_NODE_NO     (6)
#define    def_PROTOCOL_NODE_NO    (3)
#define    def_IO_TCP_SERVER_NODE_NO          (1)
#define    def_IO_TCP_CLIENT_NODE_NO          (1)
#define    def_IO_TCP_EXT_CLIENT_NODE_NO          (6)
#define    def_IO_COM_EXT_NODE_NO          (6)


#define    def_DEVICE_RFID_READER_NODE_NO      (6)


//心跳包间隔时间 unit:s
#define    def_HEART_BEAT_INTERVAL_S        (20)

#define     RFID_READER_MAX_POWER               (30)
#define     RFID_READER_MAX_SCNTIME             (255)
#define     RFID_READER_MIN_SCNTIME             (3)

#define     FIXED_EPC_LEN                       (12)
#define     FIXED_DATA_LEN                      (12)


#define     def_LOG_LEV_STRING          ("log_level")
#define     def_LOG_LEV_TRACE           ("trace")
#define     def_LOG_LEV_DEBUG           ("debug")
#define     def_LOG_LEV_INFO            ("info")
#define     def_LOG_LEV_WARN            ("warn")
#define     def_LOG_LEV_ERROR           ("error")
#define     def_LOG_LEV_FATAL           ("fatal")


#endif

