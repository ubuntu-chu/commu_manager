#include "protocol_rfid.h"

#define PRESET_VALUE (0xFFFF)
#define POLYNOMIAL  (0x8408)

static unsigned int uiCrc16Cal(unsigned char const  * pucY, unsigned char ucX)
{
    unsigned char ucI,ucJ;
    unsigned short int  uiCrcValue = PRESET_VALUE;

//  SYS_LOG("ucX = %d\n", ucX);

    for (ucI = 0; ucI < ucX; ucI++){

        uiCrcValue = uiCrcValue ^ *(pucY + ucI);
        for (ucJ = 0; ucJ < 8; ucJ++){
            if(uiCrcValue & 0x0001) {
                uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
            } else{
                uiCrcValue = (uiCrcValue >> 1);
            }
        }
    }

    return uiCrcValue;
}

//初始化规约
bool protocol_rfid::init()
{

    return true;
}

//反初始化
void protocol_rfid::uninit()
{

}

int protocol_rfid::package_aframe(char* pdata, int len)
{
    uint16  crc;

    //len中包含 地址+命令+数据
    //长度   命令数据块的长度  但不包括len本身
    outbuffer_.prependInt8(len+2);
    //地址
    outbuffer_.appendInt8(pdata[0]);
    //命令
    outbuffer_.appendInt8(pdata[1]);
    //数据
    outbuffer_.append(&pdata[2], len-2);
    //crc
    crc     = uiCrc16Cal(reinterpret_cast<const unsigned char *>(outbuffer_.peek()), outbuffer_.readableBytes());
    outbuffer_.appendInt8(crc&0x00ff);
    outbuffer_.appendInt8(crc >> 8);

    return outbuffer_.readableBytes();
}

bool protocol_rfid::process_aframe(const char * pdata, int len, int iflag)
{
    protocol::process_aframe(pdata, len, iflag);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
//函数说明: 验证一帧是否有效
//参数说明: BYTE* pData 报文数据缓冲区
//参数说明: int iDataLen 缓冲区数据长度
//参数说明: int& iPackLen 数据帧长度  当数据仍在接收时 不关心ipacklen的值
//返 回 值: < 0 数据仍在接受   = 0 一帧接收完成   < 0 一帧接收出错
//备    注:
////////////////////////////////////////////////////////////////////////////////

int  protocol_rfid::validate_aframe(const char* pdata, int len, int& ipacklen)
{
    //check frame valid
    uint16      crc;
    int     rt                  = 1;

    ipacklen                    = 0;
    if (len < (pdata[0] + 1)){
        rt                      = 0;
        goto quit;
    }
    ipacklen                    = len;
    //caculate crc
    //验证crc校验码
    crc     = uiCrc16Cal(reinterpret_cast<const unsigned char *>(pdata), len);
    if (0 != crc){
        rt                      = -1;
        goto quit;
    }

//    utils::log_binary_buf("protocol_rfid::validate_aframe", outbuffer_.peek(), outbuffer_.readableBytes());
    //对接收到的数据内容进行校验
    //地址校验
    outbuffer_.retrieveInt8();
    if (pdata[1] != outbuffer_.readInt8()){
        rt                      = -2;
        goto quit;
    }
    //命令校验
    if (pdata[2] != outbuffer_.readInt8()){
        rt                      = -3;
        goto quit;
    }
#if 0
    //命令执行状态校验
    if (pdata[3] != 0){
        rt                      = -4;
        goto quit;
    }
#endif
quit:
    LOG_INFO << "return value = " << rt << " ipacklen = " << ipacklen << " status = "
        << static_cast<uint8>(pdata[3]);
    return rt;
}

bool protocol_rfid::handle_timer(void)
{
    LOG_DEBUG;
    return true;
}

