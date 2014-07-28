#ifndef _PROTOCOL_RAW_H
#define _PROTOCOL_RAW_H

#include <includes/includes.h>
#include <protocol.h>

typedef uint32_t    sys_time_t;
typedef uint32_t    sys_tick_t;
typedef uint32_t    dev_adr_t;
typedef uint32_t    sen_adr_t;

typedef enum{
    FRAME_TYPE_CTRL = 0x00,     //
    FRAME_TYPE_HEARTBEAT,       //
    FRAME_TYPE_STATUS,          // status out
    FRAME_TYPE_DATA,            // sensor data out
}FRAME_TYPE_T;

enum{
    DOWNSTREAM      = 0,
    UPSTREAM,
};

enum{
    NO_ACK_REQUEST      = 0,
    ACK_REQUEST,
};

enum{
    NO_ACK_FRAME        = 0,
    ACK_FRAME,
};

//typedef enum{
//  FUNC_IDLE = 0x00,
//  FUNC_IDLE,
//  FUNC_IDLE,
//  FUNC_IDLE,
//  FUNC_IDLE,
//  FUNC_IDLE,
//  FUNC_IDLE,
//}FUNC_TYPE_T;

typedef struct{
    uint16_t frm_type   :   4,              // frame type;
    direction           :   1,              // direction: 1 sensor -> sink, 0 sink -> sensor;
    frm_pending         :   1,              // frame pending;   data to send for expand
    ack_req             :   1,              // ACK request;     this frame need to ack
    ack_mask            :   1,              // ACK bit. 1 for ack frame
    reserved            :   2,              //reseverd
    des_addr_mode       :   2,              // dest addressing mode;
    frm_version         :   2,              //frame version
    src_addr_mode       :   2;              //soure addressing mode
} mac_frm_ctrl_t;



typedef struct{
    uint16_t    delimiter_start;
    uint16_t    len;
    uint16_t    seq_id;
    mac_frm_ctrl_t  ctl;
    sys_time_t  time;
    dev_adr_t   dev_adr;
    sen_adr_t   sen_adr;
    uint8_t     type;
}mac_frame_t;

typedef struct{
    uint8_t     fun;
    uint8_t     sum;
    uint8_t     idex;
    uint16_t    len;
}app_frame_t;

typedef struct{
    mac_frame_t mac_frm_ptr;
    app_frame_t app_frm_ptr;
    uint8_t*    data_ptr;
}frame_ctl_t;


// BASE FILE
#define def_FRAME_PHY_LEN_LEN       1
#define def_FRAME_DELIMITER_LEN     2

#define def_FRAME_delimiter         0x5AA5
#define def_FRAME_END_delimiter     0xAA55




#define def_FRAME_LEN_LEN           2
#define def_FRAME_SEQ_LEN           2
#define def_FRAME_CTL_LEN           2

#define def_ACK_MASK            (1<<7)
#define def_ACK_NEED_MASK       (1<<6)
#define def_DATA_SEND_MASK      (1<<5)
#define def_UP_DIR_MASK         (1<<4)


#define def_FRAME_TIME_LEN          4
#define def_FRAME_DST_LEN           4
#define def_FRAME_SRC_LEN           4

#define def_FRAME_TYPE_LEN          1
#define def_FRAME_FUN_LEN           1
#define def_FRAME_SUM_PACK_LEN      1
#define def_FRAME_PACK_IDEX_LEN     1
#define def_FRAME_DATA_LEN          2

#define def_FRAME_CK_LEN            2


#define def_FRAME_MAC_LEN           def_FRAME_SEQ_LEN+def_FRAME_CTL_LEN  \
                                    +def_FRAME_TIME_LEN+def_FRAME_DST_LEN \
                                    +def_FRAME_SRC_LEN+def_FRAME_TYPE_LEN

#define def_FRAME_APP_HEAD_LEN      def_FRAME_FUN_LEN + def_FRAME_SUM_PACK_LEN


//define little endian format
#define def_BIG_ENDIAN              0
#define WORD                uint16_t
#define DWORD               uint32_t
#define BYTE                uint8_t
#define BYTE_PTR            uint8_t*

#if def_BIG_ENDIAN == 0

#define LD_WORD(ptr)        (WORD)(((WORD)*(BYTE_PTR)((ptr)+1)<<8)|(WORD)*(BYTE_PTR)(ptr))

#define LD_DWORD(ptr)       (DWORD)(((DWORD)*(BYTE_PTR)((ptr)+3)<<24)   |   \
                            ((DWORD)*(BYTE_PTR)((ptr)+2)<<16)           |   \
                            ((WORD)*(BYTE_PTR)((ptr)+1)<<8)             |   \
                            *(BYTE_PTR)(ptr))

#define ST_WORD(ptr,val)    *(BYTE_PTR)(ptr)=(BYTE)(val);                   \
                            *(BYTE_PTR)((ptr)+1)=(BYTE)((WORD)(val)>>8)

#define ST_DWORD(ptr,val)   *(BYTE_PTR)(ptr)=(BYTE)(val);                   \
                            *(BYTE_PTR)((ptr)+1)=(BYTE)((WORD)(val)>>8);    \
                            *(BYTE_PTR)((ptr)+2)=(BYTE)((DWORD)(val)>>16);  \
                            *(BYTE_PTR)((ptr)+3)=(BYTE)((DWORD)(val)>>24)
#else
#define LD_DWORD(ptr)       (DWORD)(((DWORD)*(BYTE_PTR)((ptr)+0)<<24)   |   \
                            ((DWORD)*(BYTE_PTR)((ptr)+1)<<16)           |   \
                            ((WORD)*(BYTE_PTR)((ptr)+2)<<8)             |   \
                            *(BYTE_PTR)((ptr)+3))

#define ST_WORD(ptr,val)    *(BYTE_PTR)((ptr)+0)=(BYTE)((WORD)(val)>>8);    \
                            *(BYTE_PTR)((ptr)+1)=(BYTE)(val)

#define ST_DWORD(ptr,val)   *(BYTE_PTR)((ptr)+0)=(BYTE)((DWORD)(val)>>24);  \
                            *(BYTE_PTR)((ptr)+1)=(BYTE)((DWORD)(val)>>16);  \
                            *(BYTE_PTR)((ptr)+2)=(BYTE)((WORD)(val)>>8);    \
                            *(BYTE_PTR)((ptr)+3)=(BYTE)(val)
#endif


#define     def_PROTOCOL_MAC_NAME           ("raw-mac")

class protocol_mac:public protocol{
public:
    protocol_mac():protocol(def_PROTOCOL_MAC_NAME){}
    virtual ~protocol_mac(){}

    //初始化规约
    virtual bool init();
    //反初始化
    virtual void uninit();

    virtual int package_aframe(char* pdata, int len);
    virtual bool process_aframe(const char * pdata, int len, int iflag = 0);

    ////////////////////////////////////////////////////////////////////////////////
    //函数说明: 验证一帧是否有效
    //参数说明: BYTE* pData 报文数据缓冲区
    //参数说明: int iDataLen 缓冲区数据长度
    //参数说明: int& iPackLen 数据帧长度  iPackLen > 0:ok;  ==0:no end;  <0 error frame
    //返 回 值: virtual int 帧起始位置在数据缓冲区的偏移
    //备    注:
    ////////////////////////////////////////////////////////////////////////////////
    virtual int  validate_aframe(const char* pdata, int len, int& ipacklen);

    virtual bool handle_timer(void);

//-----------------------------------------------------------------

    void frm_ctl_init(frame_ctl_t *pfrm_ctl, mac_frm_ctrl_t frm_ctl, uint8 total, uint8 index, uint8 func_code, uint8 *pbuf, uint16 len);
    mac_frm_ctrl_t mac_frm_ctrl_init(uint8 ack, uint8 dir, uint8 ack_req, uint8 frm_type);
    int8 frm_ctrl_unpack(uint8_t* pbuf, uint16 len, frame_ctl_t *pfrm_ctl);



//private:

};















#endif

