#include "com.h"
#include <utils.h>

struct baud_setting{
    int     m_baud;
    int     m_value;
};

const struct baud_setting t_serial_baud_setting[] = {
    {921600,    B921600},
    {460800,    B460800},
    {230400,    B230400},
    {115200,    B115200},
    {57600 ,    B57600 },
    {38400 ,    B38400 },
    {19200 ,    B19200 },
    {9600  ,    B9600  },
    {4800  ,    B4800  },
    {2400  ,    B2400  },
    {1200  ,    B1200  },
    {300   ,    B300   },
};

int dev_conf_save(int fd, struct termios *ptermios){

    if (tcgetattr(fd, ptermios) != 0){
        perror("tcgetattr error!\n");
        return -1;
    }

    return 0;
}

int dev_conf_restore(int fd, struct termios *ptermios){

    if (tcsetattr(fd, TCSANOW, ptermios) != 0){
        perror("tcsetattr error!\n");
        return -1;
    }

    return 0;
}

int dev_conf(int fd, int baud, int databits, int stopbits, int parity){

    struct termios newtio;
    unsigned int i;

    tcgetattr(fd, &newtio);
    bzero(&newtio, sizeof(newtio));

    //baud
    for (i = 0;  i < sizeof(t_serial_baud_setting)/sizeof(t_serial_baud_setting[0]);  i++) {
        if (baud == t_serial_baud_setting[i].m_baud){
            cfsetispeed(&newtio, t_serial_baud_setting[i].m_value);
            cfsetospeed(&newtio, t_serial_baud_setting[i].m_value);
            break;
        }
    }
    if (i >= sizeof(t_serial_baud_setting)/sizeof(t_serial_baud_setting[0])){
        printf("dev baud param invalid!\n");
        return -1;
    }

    //setting   c_cflag
    newtio.c_cflag &= ~CSIZE;
    /*设置数据位数 */
    switch (databits) {
    case 7:
        newtio.c_cflag |= CS7;  //7位数据位
        break;
    case 8:
        newtio.c_cflag |= CS8;  //8位数据位
        break;
    default:
        newtio.c_cflag |= CS8;
        break;
    }
    //设置校验
    switch (parity){
    case 'n':
    case 'N':
        newtio.c_cflag &= ~PARENB;  /* Clear parity enable */
        newtio.c_iflag &= ~INPCK;   /* Enable parity checking */
        break;
    case 'o':
    case 'O':
        newtio.c_cflag |= (PARODD | PARENB);    /* 设置为奇效验 */
        newtio.c_iflag |= INPCK;    /* Disnable parity checking */
        break;
    case 'e':
    case 'E':
       newtio.c_cflag |= PARENB;    /* Enable parity */
       newtio.c_cflag &= ~PARODD;   /* 转换为偶效验 */
       newtio.c_iflag |= INPCK; /* Disnable parity checking */
        break;
    case 'S':
    case 's':           /*as no parity */
        newtio.c_cflag &= ~PARENB;
        newtio.c_cflag &= ~CSTOPB;
        break;

    default:
       newtio.c_cflag &= ~PARENB;   /* Clear parity enable */
       newtio.c_iflag &= ~INPCK;    /* Enable parity checking */
       break;
    }
    //设置停止位
    switch (stopbits){
    case 1:
       newtio.c_cflag &= ~CSTOPB;   //1
       break;
    case 2:
       newtio.c_cflag |= CSTOPB;    //2
        break;
    default:
       newtio.c_cflag &= ~CSTOPB;
       break;
    }

/*
 *
 * 在串口编程模式下，open未设置O_NONBLOCK或O_NDELAY的情况下。c_cc[VTIME]和c_cc[VMIN]影响read函数的返回。
 * 若在open或fcntl设置了O_NDELALY或O_NONBLOCK标志，read调用不会阻塞而是立即返回，那么VTIME和VMIN就没有
 * 意义，效果等同于与把VTIME和VMIN都设为了0。
 *
 * VTIME:定义等待的时间，单位是百毫秒(通常是一个8位的unsigned char变量，取值不能大于cc_t)。
 * VMIN:定义了要求等待的最小字节数，(不是要求读的字节数——read()的第三个参数才是指定要求读的最大字节数)，
 * 这个字节数可能是0
 *
 * c_cc[VMIN] > 0, c_cc[VTIME] == 0 如果VTIME取0，VMIN定义了要求等待读取的最小字节数。
 * 函数read()只有在读取了VMIN个字节的数据或者收到一个信号的时候才返回。
 *
 * c_cc[VMIN] == 0, c_cc[VTIME] > 0 如果VMIN取0，VTIME定义了即使没有数据可以读取，read()函数返回前也要
 * 等待几百毫秒的时间量。这时，read()函数不需要像其通常情况那样要遇到一个文件结束标志才返回0。
 *
 * c_cc[VMIN] > 0, c_cc[VTIME] > 0 如果VTIME和VMIN都不取0，VTIME定义的是当接收到第一个字节的数据后开始
 * 计算等待的时间量。如果当调用read函数
 * 时可以得到数据，计时器马上开始计时。如果当调用read函数时还没有任何数据可读，则等接收到第一个字节的数据后，
 * 计时器开始计时。函数read可能会在读取到VMIN个字节的数据后返回，也可能在计时完毕后返回，这主要取决于哪个
 * 条件首先实现。不过函数至少会读取到一个字节的数据，因为计时器是在读取到第一个数据时开始计时的。
 *
 * c_cc[VMIN] == 0, c_cc[VTIME] == 0 如果VTIME和VMIN都取0，即使读取不到任何数据，函数read也会立即返回。
 * 同时，返回值0表示read函数不需要等待文件结束标志就返回了。
 *
 */

    newtio.c_cc[VTIME] = 0;                 //注意此处的设置
    newtio.c_cc[VMIN] = 1;

    newtio.c_cflag |= (CLOCAL | CREAD);
    //newtio.c_oflag |= OPOST;
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
    //IO设置为原始模式 ICRNL 将输入的CR转换为NL IXON 使起动/停止输出控制流起作用
    //newtio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_oflag &=~ OPOST;

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &newtio) != 0) {
        perror("serial dev config fail");
        return -1;
    }
    return 0;
}

int dev_open(const char *ppath)
{
    int fd;

    //non - blocking open
    fd = open(ppath, O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC);

    if (fd < 0){
        LOG_ERROR << "open device: " << ppath << " failed\n";
    }

    return fd;
}

void dev_block(int fd){
    int flags;

    flags = fcntl(fd,F_GETFL,0);
    flags &= ~O_NONBLOCK;
    fcntl(fd,F_SETFL,flags);
}


void dev_nonblock(int fd){
    int flags;

    flags = fcntl(fd,F_GETFL,0);
    flags |= O_NONBLOCK;
    fcntl(fd,F_SETFL,flags);
}

int dev_close(int fd){

    if (fd < 0){
        return -1;
    }
    tcflush(fd, TCIOFLUSH);
    close(fd);

    return 0;
}

int dev_write(int fd, const char *str, int len){

    int n       = 0;
    int tot_n   = 0;

    while (len){

        n = write(fd, str+tot_n, len);
        if (n < 0){
            if (errno == EINTR){
                continue;
            }else {
                break;
            }
        }
        tot_n   += n;
        len     -= n;
    }

    return tot_n;
}

com::com(EventLoop* loop, const char *name, io_node *pio_node) :
        io_base(pio_node), loop_(loop)
{
    LOG_INFO << "IO_TYPE_EXT_COM create; name = " << name;
}

bool com::init(void)
{
    loop_->runInLoop(boost::bind(&com::_init, this));

    return true;
}

//初始化通信介质
bool com::_init(void)
{
    io_com_ext_node *pio_com_ext_node =
            reinterpret_cast<io_com_ext_node *>(const_cast<io_node *>(pio_node_));
    char *com_path;
    int     bps, stop, bits, parity;
    char parity_print;

	pio_com_ext_node->ios_get(&com_path, bps, stop, bits, parity);

	parity_print        = parity;
    LOG_INFO << "device: " << com_path << " bps: " << bps << " stop: " << stop
            << " bits: " << bits << " parity: " << parity_print;

    fd_                                  = dev_open(com_path);
    if (dev_conf_save(fd_, &termios_) < 0){
        return false;
    }
    if (dev_conf(fd_, bps, bits, stop, parity) < 0) {
        LOG_ERROR << com_path << "termios set failed!";
        close(fd_);
        return false;
    }

    channel_.reset(new Channel(loop_, fd_));
    channel_->setReadCallback(
        boost::bind(&com::handle_read, this, _1));
    LOG_INFO << "enable channel readable begin";
    channel_->enableReading();
    LOG_INFO << "enable channel readable end";

    return true;
}

bool com::uninit(void)
{
    loop_->runInLoop(boost::bind(&com::_uninit, this));

    return true;
}

//反初始化
bool com::_uninit(void)
{
    if (fd_ < 0){
        return true;
    }
    dev_conf_restore(fd_, &termios_);
    dev_close(fd_);

    return true;
}

//向通道写报文
int com::send_data(char *pdata, size_t len)
{
    loop_->runInLoop(boost::bind(&com::_send_data, this, pdata, len));

    return len;
}

int com::_send_data(char *pdata, size_t len)
{
    send_status_end(dev_write(fd_, pdata, len));

    return len;
}

void com::handle_read(Timestamp receiveTime)
{
    char buf[1000];
    ssize_t len;

    while (1){
        len = read(fd_, (char *) buf, sizeof(buf));
        if (len < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    io_base::on_read(buf, len, 0);
}


